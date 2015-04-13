/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/soundcard.h>
#include <iostream>
#include "tone_gen.h"
#include "log.h"
#include "sys_settings.h"
#include "user.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"
#include "audio_device.h"

// Number of samples read at once from the wav file
#define NUM_SAMPLES_PER_TURN	1024

// Duration of one turn in ms
#define DURATION_TURN	(NUM_SAMPLES_PER_TURN * 1000 / wav_info.samplerate)


// Main function for play thread
void *tone_gen_play(void *arg) {
	ui->add_prohibited_thread();
	
	t_tone_gen *tg = (t_tone_gen *)arg;
	tg->play();
	
	ui->remove_prohibited_thread();
	
	return NULL;
}

t_tone_gen::t_tone_gen(const string &filename, const t_audio_device &_dev_tone) :
		dev_tone(_dev_tone),
		sema_finished(0)
{
	string f;

	wav_file = NULL;
	aio = 0;
	valid = false;
	data_buf = NULL;
	thr_play = NULL;
	loop = false;
	pause = 0;

	if (filename.size() == 0) return;

	// Add share directory to filename
	if (filename[0] != '/') {
		f = sys_config->get_dir_share();
		f += "/";
		f += filename;
	} else {
		f = filename;
	}

	wav_filename = f;

	memset(&wav_info, 0, sizeof(SF_INFO));
	wav_file = sf_open(f.c_str(), SFM_READ, &wav_info);
	if (!wav_file) {
		string msg("Cannot open ");
		msg += f;
		log_file->write_report(msg, "t_tone_gen::t_tone_gen",
			LOG_NORMAL, LOG_WARNING);
		ui->cb_display_msg(msg, MSG_WARNING);
		return;
	}

	log_file->write_header("t_tone_gen::t_tone_gen");
	log_file->write_raw("Opened ");
	log_file->write_raw(f);
	log_file->write_endl();
	log_file->write_footer();

	valid = true;
	stop_playing = false;
}

t_tone_gen::~t_tone_gen() {
	if (wav_file) {
		sf_close(wav_file);
	}
	if (aio) {
		MEMMAN_DELETE(aio);
		delete aio;
	}
	aio = 0;
	if (data_buf) {
		MEMMAN_DELETE_ARRAY(data_buf);
		delete [] data_buf;
	}
	if (thr_play) {
		MEMMAN_DELETE(thr_play);
		delete thr_play;
	}
	
	log_file->write_report("Deleted tone generator.",
		"t_tone_gen::~t_tone_gen");
}

bool t_tone_gen::is_valid(void) const {
	return valid;
}

void t_tone_gen::play(void) {
	if (!valid) {
		log_file->write_report(
			"Tone generator is invalid. Cannot play tone",
			"t_tone_gen::play", LOG_NORMAL, LOG_WARNING);
		sema_finished.up();
		return;
	}

	aio = t_audio_io::open(dev_tone, true, false, true, wav_info.channels,
		SAMPLEFORMAT_S16, wav_info.samplerate, false);
	if (!aio) {
		string msg("Failed to open sound card: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_tone_gen::play",
			LOG_NORMAL, LOG_WARNING);
		ui->cb_display_msg(msg, MSG_WARNING);
		sema_finished.up();
		return;
	}
	
	log_file->write_report("Start playing tone.",
		"t_tone_gen::play");

	do {
		// Each samples consists of #channels shorts
		data_buf = new short[NUM_SAMPLES_PER_TURN * wav_info.channels];
		MEMMAN_NEW_ARRAY(data_buf);

		sf_count_t frames_read = NUM_SAMPLES_PER_TURN;
		while (frames_read == NUM_SAMPLES_PER_TURN) {
			if (stop_playing) break;

			// Play sample
			frames_read = sf_readf_short(wav_file, data_buf, NUM_SAMPLES_PER_TURN);
			if (frames_read > 0) {
				aio->write((unsigned char*)data_buf, 
					frames_read * wav_info.channels * 2);
			}
		}

		MEMMAN_DELETE_ARRAY(data_buf);
		delete [] data_buf;
		data_buf = NULL;

		if (stop_playing) break;

		// Pause between repetitions
		if (loop) {
			// Play silence
			if (pause > 0) {
				data_buf = new short[NUM_SAMPLES_PER_TURN * wav_info.channels];
				MEMMAN_NEW_ARRAY(data_buf);
				memset(data_buf, 0, NUM_SAMPLES_PER_TURN * wav_info.channels * 2);

				for (int i = 0; i < pause; i += DURATION_TURN) {
					aio->write((unsigned char*)data_buf, 
						NUM_SAMPLES_PER_TURN * wav_info.channels * 2);
					if (stop_playing) break;
				}

				MEMMAN_DELETE_ARRAY(data_buf);
				delete [] data_buf;
				data_buf = NULL;
			}

			if (stop_playing) break;

			// Set file pointer back to start of data
			sf_seek(wav_file, 0, SEEK_SET);
		}
	} while (loop);
	
	log_file->write_report("Tone ended.",
		"t_tone_gen::play_tone");

	sema_finished.up();
}

void t_tone_gen::start_play_thread(bool _loop, int _pause) {
	loop = _loop;
	pause = _pause;
	thr_play = new t_thread(tone_gen_play, this);
	MEMMAN_NEW(thr_play);
	thr_play->detach();
}

void t_tone_gen::stop(void) {
	log_file->write_report("Stopping tone.",
		"t_tone_gen::stop");

	if (stop_playing) {
		log_file->write_report("Tone has stopped already.",
			"t_tone_gen::stop");
		return;
	}

	// This will stop the playing thread.
	stop_playing = true;
	
	// The semaphore will be upped by the playing thread as soon 
	// as playing finishes.
	sema_finished.down();
	
	log_file->write_report("Tone stopped.",
		"t_tone_gen::stop");

	if (aio) {
		MEMMAN_DELETE(aio);
		delete aio;
		aio = 0;
	}
}


