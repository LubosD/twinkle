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

#include "audio_device.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "sys_settings.h"
#include "translator.h"
#include "log.h"
#include "userintf.h"
#include "util.h"
#include "audits/memman.h"

#ifdef HAVE_LIBASOUND
#include <alsa/asoundlib.h>
#endif

t_audio_io* t_audio_io::open(const t_audio_device& dev, bool playback, bool capture, bool blocking, int channels, t_audio_sampleformat format, int sample_rate, bool short_latency)
{
	t_audio_io* aio;
	
	if(dev.type == t_audio_device::OSS) {
		aio = new t_oss_io();
		MEMMAN_NEW(aio);
#ifdef HAVE_LIBASOUND
	} else if (dev.type == t_audio_device::ALSA) {
		aio = new t_alsa_io();
		MEMMAN_NEW(aio);
#endif
	} else {
		string msg("Audio device not implemented");
		log_file->write_report(msg, "t_audio_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		return 0;
	}
	if (aio->open(dev.device, playback, capture, blocking, channels, format, 
			sample_rate, short_latency)) {
		return aio;
	} else {
		string msg("Open audio device failed");
		log_file->write_report(msg, "t_audio_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		MEMMAN_DELETE(aio);
		delete aio;
		return 0L;
	}
}

bool t_audio_io::validate(const t_audio_device& dev, bool playback, bool capture) {
	t_audio_io *aio = open(dev, playback, capture, false, 1, SAMPLEFORMAT_S16, 8000, true);
	
	if (aio) {
		MEMMAN_DELETE(aio);
		delete aio;
		return true;
	}
	
	return false;
}

t_audio_io::~t_audio_io() {}

int t_audio_io::get_sample_rate(void) const {
	return _sample_rate;
}

bool t_audio_io::open(const string& device, bool playback, bool capture, bool blocking, int channels, t_audio_sampleformat format, int sample_rate, bool short_latency)
{
	_sample_rate = sample_rate;
	return true;
}

t_oss_io::t_oss_io() : fd(-1), play_buffersize(0), rec_buffersize(0) {
}

t_oss_io::~t_oss_io()
{
	if (fd > 0) {
		int arg = 0;
		ioctl(fd, SNDCTL_DSP_RESET, &arg);
		close(fd);
	}
	fd = -1;
}

bool t_oss_io::open(const string& device, bool playback, bool capture, bool blocking, int channels, t_audio_sampleformat format, int sample_rate, bool short_latency) 
{
	t_audio_io::open(device, playback, capture, blocking, channels, format, sample_rate,
		short_latency);

	int mode = 0;
	int status;
	
	log_file->write_header("t_oss_io::open", LOG_NORMAL);
	log_file->write_raw("Opening OSS device: ");
	log_file->write_raw(device);
	log_file->write_endl();
	if (playback) log_file->write_raw("play\n");
	if (capture) log_file->write_raw("capture\n");
	log_file->write_footer();
	
	assert (playback || capture);
	if (playback && capture) mode |= O_RDWR;
	else if (playback) mode |= O_WRONLY;
	else if (capture) mode |= O_RDONLY;
	
	// On some systems opening the audio devices blocks if another
	// process or thread has opened it already. To prevent a deadlock
	// first try to open the device in non-blocking mode.
	// If the device is still open by another twinkle thread then that
	// is a bug, but this way at least non deadlock is caused.
	if(blocking) {
		fd = ::open(device.c_str(), mode | O_NONBLOCK);
		if (fd == -1) {
			string msg("OSS audio device open failed: ");
			msg += get_error_str(errno);
			log_file->write_report(msg, "t_oss_io::open",
				LOG_NORMAL, LOG_CRITICAL);
			return false;
		} else {
			close(fd);
			fd = -1;
		}
	} else mode |= O_NONBLOCK;
	
	fd = ::open(device.c_str(), mode);
	if (fd < 0) {
		string msg("OSS audio device open failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		return false;
	}
	
	// Full duplex
	if (playback && capture) {
		status = ioctl(fd, SNDCTL_DSP_SETDUPLEX, 0);
		if (status == -1) {
			string msg("SNDCTL_DSP_SETDUPLEX ioctl failed: ");
			msg += get_error_str(errno);
			log_file->write_report(msg, "t_oss_io::open",
				LOG_NORMAL, LOG_CRITICAL);
			ui->cb_display_msg(TRANSLATE("Sound card cannot be set to full duplex."),
				MSG_CRITICAL);
			close(fd);
			return false;
		}
	}
	// Set fragment size
	int arg;
	if (short_latency) {
		switch (sys_config->get_oss_fragment_size()) {
		case 16:
			arg = 0x00ff0004; // 255 buffers of 2^4 bytes each
			break;		
		case 32:
			arg = 0x00ff0005; // 255 buffers of 2^5 bytes each
			break;
		case 64:
			arg = 0x00ff0006; // 255 buffers of 2^5 bytes each
			break;
		case 128:
			arg = 0x00ff0007; // 255 buffers of 2^7 bytes each
			break;
		case 256:
			arg = 0x00ff0008; // 255 buffers of 2^8 bytes each
			break;
		default:
			arg = 0x00ff0007; // 255 buffers of 2^7 bytes each
		}
	} else {
		arg = 0x00ff000a; // 255 buffers of 2^10 bytes each
	}
	status = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &arg);
	if (status == -1) {
		string msg("SNDCTL_DSP_FRAGMENT ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(TRANSLATE("Cannot set buffer size on sound card."),
			MSG_CRITICAL);
		close(fd);
		return false;
	}
	// Channels
	arg = channels;
	status = ioctl(fd, SNDCTL_DSP_CHANNELS, &arg);
	if (status == -1) {
		string msg("SNDCTL_DSP_CHANNELS ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		msg = TRANSLATE("Sound card cannot be set to %1 channels.");
		msg = replace_first(msg, "%1", int2str(channels));
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}
	if (arg != channels) {
		log_file->write_report("Unable to set channels",
			"t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		string msg = "Sound card cannot be set to ";
		msg = TRANSLATE("Sound card cannot be set to %1 channels.");
		msg = replace_first(msg, "%1", int2str(channels));
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}
	// Sample format
	int fmt;
	switch (format) {
		case SAMPLEFORMAT_S16:
#ifdef WORDS_BIGENDIAN
			fmt = AFMT_S16_BE;
#else
			fmt = AFMT_S16_LE;
#endif
			break;
		case SAMPLEFORMAT_U16:
#ifdef WORDS_BIGENDIAN
			fmt = AFMT_U16_BE;
#else
			fmt = AFMT_U16_LE;
#endif
			break;
		case SAMPLEFORMAT_S8:
			fmt = AFMT_S8;
			break;
		case SAMPLEFORMAT_U8:
			fmt = AFMT_U8;
			break;
		default:
			fmt = 0; // fail
	}
	arg = fmt;
	status = ioctl(fd, SNDCTL_DSP_SETFMT, &arg);
	if (status == -1) {
		string msg("SNDCTL_DSP_SETFMT ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(TRANSLATE("Cannot set sound card to 16 bits recording."),
			MSG_CRITICAL);
		return false;
	}

	arg = fmt;
  	status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
	if (status == -1) {
		string msg("SOUND_PCM_WRITE_BITS ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		ui->cb_display_msg(TRANSLATE("Cannot set sound card to 16 bits playing."),
			MSG_CRITICAL);
		return false;
	}

	// Sample rate
	arg = sample_rate;
	status = ioctl(fd, SNDCTL_DSP_SPEED, &arg);
	if (status == -1) {
		string msg("SNDCTL_DSP_SPEED ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		msg = TRANSLATE("Cannot set sound card sample rate to %1");
		msg = replace_first(msg, "%1", int2str(sample_rate));
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}
	
	play_buffersize = rec_buffersize = 0;
	if (playback) play_buffersize = get_buffer_space(false);
	if (capture) rec_buffersize = get_buffer_space(true);
	return true;
}

void t_oss_io::enable(bool enable_playback, bool enable_recording) {
	int arg, status;
	arg = enable_recording ? PCM_ENABLE_INPUT : 0;
	arg |= enable_playback ? PCM_ENABLE_OUTPUT : 0;
	status = ioctl(fd, SNDCTL_DSP_SETTRIGGER, &arg);
	if (status == -1) {
		string msg("SNDCTL_DSP_SETTRIGGER ioctl failed: ");
		msg += get_error_str(errno);
		log_file->write_report(msg, "t_oss_io::enable",
			LOG_NORMAL, LOG_CRITICAL);
	}
}

void t_oss_io::flush(bool playback_buffer, bool recording_buffer) {
	for (int i = 0; i < 2; i++) {
		// i == 0: flush playback buffer, 1: flush recording buffer
		if (i == 0 && playback_buffer || i == 1 && recording_buffer) {
			int skip_bytes = ( (i==0) ? play_buffersize : 
				rec_buffersize) - get_buffer_space(i == 1);
			if(skip_bytes <= 0) continue;
			unsigned char *trash = new unsigned char[skip_bytes];
			MEMMAN_NEW_ARRAY(trash);
			read(trash, skip_bytes);
			MEMMAN_DELETE_ARRAY(trash);
			delete [] trash;
		}
	}
}

int t_oss_io::get_buffer_space(bool is_recording_buffer)
{
	audio_buf_info dsp_info;
	int status = ioctl(fd, is_recording_buffer ? SNDCTL_DSP_GETISPACE :
		SNDCTL_DSP_GETOSPACE, &dsp_info);
	if (status == -1) return 0;
	return dsp_info.bytes;
}

int t_oss_io::get_buffer_size(bool is_recording_buffer)
{
	if (is_recording_buffer) return rec_buffersize;
	else return play_buffersize;
}

bool t_oss_io::play_buffer_underrun(void) {
	return get_buffer_space(false) >= get_buffer_size(false);
}


int t_oss_io::read(unsigned char* buf, int len) {
	return ::read(fd, buf, len);
}

int t_oss_io::write(const unsigned char* buf, int len) {
	return ::write(fd, buf, len);
}


#ifdef HAVE_LIBASOUND
t_alsa_io::t_alsa_io() : pcm_play_ptr(0), pcm_rec_ptr(0), play_framesize(1), rec_framesize(1), 
	play_buffersize(0), rec_buffersize(0) {
}

t_alsa_io::~t_alsa_io() {
	if (pcm_play_ptr) {
		log_file->write_header("t_alsa_io::~t_alsa_io", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("snd_pcm_close, handle = ");
		log_file->write_raw(ptr2str(pcm_play_ptr));
		log_file->write_endl();
		log_file->write_footer();
		
		// Without the snd_pcm_hw_free, snd_pcm_close sometimes fails.
		snd_pcm_hw_free(pcm_play_ptr);
		snd_pcm_close(pcm_play_ptr);
		pcm_play_ptr = 0;
	}
	if (pcm_rec_ptr) {
		log_file->write_header("t_alsa_io::~t_alsa_io", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("snd_pcm_close, handle = ");
		log_file->write_raw(ptr2str(pcm_rec_ptr));
		log_file->write_endl();
		log_file->write_footer();
		
		snd_pcm_hw_free(pcm_rec_ptr);
		snd_pcm_close(pcm_rec_ptr);
		pcm_rec_ptr = 0;
	}
}


bool t_alsa_io::open(const string& device, bool playback, bool capture, bool blocking, int channels, t_audio_sampleformat format, int sample_rate, bool short_latency) 
{
	t_audio_io::open(device, playback, capture, blocking, channels, format, sample_rate,
		short_latency);
		
	int mode = 0;
	string msg;
	
	this->short_latency = short_latency;
	
	const char* dev = device.c_str();
	if(dev[0] == 0) dev = "default";
	
	log_file->write_header("t_alsa_io::open", LOG_NORMAL);
	log_file->write_raw("Opening ALSA device: ");
	log_file->write_raw(dev);
	log_file->write_endl();
	if (playback) log_file->write_raw("play\n");
	if (capture) log_file->write_raw("capture\n");
	log_file->write_footer();
	
	if (playback && capture) {
		// Full duplex: Perform the operation in two steps
		if (!open(device, true, false, blocking, channels, format, 
				sample_rate, short_latency))
			return false;
		if (!open(device, false, true, blocking, channels, format, 
				sample_rate, short_latency))
			return false;
			
		return true;
	}
	snd_pcm_t* pcm_ptr;
	
#define HANDLE_ALSA_ERROR(func) string msg(func); msg += " failed: "; msg += snd_strerror(err); \
	log_file->write_report(msg, "t_alsa_io::open", LOG_NORMAL, LOG_CRITICAL); msg = TRANSLATE("Opening ALSA driver failed") + ": " + msg; \
	ui->cb_display_msg(msg, MSG_CRITICAL); if(pcm_ptr) snd_pcm_close(pcm_ptr); return false;
	
	if (!blocking) mode = SND_PCM_NONBLOCK;
	
	int err = snd_pcm_open(&pcm_ptr, dev, playback ? SND_PCM_STREAM_PLAYBACK :
			SND_PCM_STREAM_CAPTURE, mode);
	if (err < 0) {
		string msg("snd_pcm_open failed: ");
		msg += snd_strerror(err);
		log_file->write_report(msg, "t_alsa_io::open",
			LOG_NORMAL, LOG_CRITICAL);
		msg = "Cannot open ALSA driver for PCM ";

		if (playback) {
			msg = TRANSLATE("Cannot open ALSA driver for PCM playback");
		} else {
			msg = TRANSLATE("Cannot open ALSA driver for PCM capture");
		}
		msg += ": ";
		msg += snd_strerror(err);
		ui->cb_display_msg(msg, MSG_CRITICAL);
		return false;
	}
	
	log_file->write_header("t_alsa_io::open", LOG_NORMAL, LOG_DEBUG);
	log_file->write_raw("snd_pcm_open succeeded, handle = ");
	log_file->write_raw(ptr2str(pcm_ptr));
	log_file->write_endl();
	log_file->write_footer();
	
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	
	// Set HW params
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_hw_params_malloc");
	}
	MEMMAN_NEW(hw_params);
				
	if ((err = snd_pcm_hw_params_any (pcm_ptr, hw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_hw_params_any");
	}

	if ((err = snd_pcm_hw_params_set_access (pcm_ptr, hw_params,
			SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		HANDLE_ALSA_ERROR("snd_pcm_hw_params_set_access");
	}
	
	snd_pcm_format_t fmt;
	int sample_bits;
	switch (format) {
		case SAMPLEFORMAT_S16:
#ifdef WORDS_BIGENDIAN
			fmt = SND_PCM_FORMAT_S16_BE;
#else
			fmt = SND_PCM_FORMAT_S16_LE;
#endif
			sample_bits = 16;
			break;
		case SAMPLEFORMAT_U16:
#ifdef WORDS_BIGENDIAN
			fmt = SND_PCM_FORMAT_U16_BE;
#else
			fmt = SND_PCM_FORMAT_U16_LE;
#endif		
			sample_bits = 16;
			break;
		case SAMPLEFORMAT_S8:
			fmt = SND_PCM_FORMAT_S8;
			sample_bits = 8;
			break;
		case SAMPLEFORMAT_U8:
			fmt = SND_PCM_FORMAT_U8;
			sample_bits = 8;
			break;
		default:
			{HANDLE_ALSA_ERROR("invalid sample format");}
	}
	
	if ((err = snd_pcm_hw_params_set_format (pcm_ptr, hw_params, fmt)) < 0) {
		string s("snd_pcm_hw_params_set_format(");
		s += int2str(fmt);
		s += ")";
		HANDLE_ALSA_ERROR(s);
	}
	
	// Some sound cards do not support the exact sample rate specified in the
	// wav files. Still we first try to set the exact sample rate instead of
	// specifying the 3rd parameter as -1 to choose an approximate.
	// For some reason on my first sound card that supports the exact rate,
	// I get mono sound when the -1 parameter is specified.
	if ((err = snd_pcm_hw_params_set_rate (pcm_ptr, hw_params, sample_rate, 0)) < 0) {
		msg = "Cannot set soundcard to exact sample rate of ";
		msg += int2str(sample_rate);
		msg += ".\nThe card will choose a lower approximate rate.";
		log_file->write_report(msg, "t_alsa_io::open", LOG_NORMAL, LOG_WARNING);
		
		if ((err = snd_pcm_hw_params_set_rate (pcm_ptr, hw_params, sample_rate, -1)) < 0) {
			string s("snd_pcm_hw_params_set_rate(");
			s += int2str(sample_rate);
			s += ")";
			HANDLE_ALSA_ERROR(s);
		}
	}
	
	// Read back card rate for reporting in the log file.
	unsigned int card_rate;
	int card_dir;
	snd_pcm_hw_params_get_rate(hw_params, &card_rate, &card_dir);
	
	if ((err = snd_pcm_hw_params_set_channels (pcm_ptr, hw_params, channels)) < 0) {
		string s("snd_pcm_hw_params_set_channels(");
		s += int2str(channels);
		s += ")";
		HANDLE_ALSA_ERROR(s);
	}
	
	// Note: The buffersize is in FRAMES, not BYTES (one frame = sizeof(sample) * channels)
	snd_pcm_uframes_t buffersize;
	unsigned int periods = 8; // Double buffering
	int dir = 1;
	
	// Set the size of one period in samples
	if (short_latency) {
		if (playback) {
			buffersize = sys_config->get_alsa_play_period_size();
		} else {
			buffersize = sys_config->get_alsa_capture_period_size();
		}
	} else {
		buffersize = 1024;
	}
	if ((err = snd_pcm_hw_params_set_period_size_near (pcm_ptr, hw_params, 
			&buffersize, &dir)) < 0) 
	{
		HANDLE_ALSA_ERROR("snd_pcm_hw_params_set_period_size_near");
	}
	
	// The number of periods determines the ALSA application buffer size.
	// This size must be larger than the jitter buffer.
	// TODO: use some more sophisticated algorithm here: read back the period
	//       size and calculate the number of periods needed (only in the
	//       short latency case)?
	if (buffersize <= 64) {
		periods *= 8;
	} else if (buffersize <= 256) {
		periods *= 4;
	}
	
	dir = 1;
	if ((err = snd_pcm_hw_params_set_periods(pcm_ptr, hw_params, periods, dir)) < 0) {
		if ((err = snd_pcm_hw_params_set_periods_near(pcm_ptr, hw_params, 
				&periods, &dir)) < 0) 
		{
			HANDLE_ALSA_ERROR("snd_pcm_hw_params_set_periods");
		}
	}
	
	if ((err = snd_pcm_hw_params (pcm_ptr, hw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_hw_params");
	}
	
	// Find out if the sound card supports pause functionality
	can_pause = (snd_pcm_hw_params_can_pause(hw_params) == 1);

	MEMMAN_DELETE(hw_params);
	snd_pcm_hw_params_free(hw_params);
	
	// Set SW params
	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_sw_params_malloc");
	}
	MEMMAN_NEW(sw_params);
	
	if ((err = snd_pcm_sw_params_current (pcm_ptr, sw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_sw_params_current");
	}
	if ((err = snd_pcm_sw_params (pcm_ptr, sw_params)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_sw_params");
	}
	
	MEMMAN_DELETE(sw_params);
	snd_pcm_sw_params_free(sw_params);
	
	if ((err = snd_pcm_prepare (pcm_ptr)) < 0) {
		HANDLE_ALSA_ERROR("snd_pcm_prepare");
	}
	if (playback) {
		pcm_play_ptr = pcm_ptr;
		play_framesize = (sample_bits / 8) * channels;
		play_buffersize = (int)buffersize * periods * play_framesize;
		play_periods = periods;
		
		log_file->write_header("t_alsa_io::open", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("ALSA playback buffer settings.\n");
		log_file->write_raw("Rate = ");
		log_file->write_raw(card_rate);
		log_file->write_raw(" frames/sec\n");
		log_file->write_raw("Frame size = ");
		log_file->write_raw(play_framesize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Periods = ");
		log_file->write_raw(play_periods);
		log_file->write_endl();
		log_file->write_raw("Period size = ");
		log_file->write_raw(buffersize * play_framesize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Buffer size = ");
		log_file->write_raw(play_buffersize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Can pause: ");
		log_file->write_bool(can_pause);
		log_file->write_endl();
		log_file->write_footer();
	} else {
		// Since audio_rx checks buffer before reading, start manually
		if ((err = snd_pcm_start(pcm_ptr)) < 0) {
			HANDLE_ALSA_ERROR("snd_pcm_start");
		}
		
		pcm_rec_ptr = pcm_ptr;
		rec_framesize = (sample_bits / 8) * channels;
		rec_buffersize = (int)buffersize * periods * rec_framesize;
		rec_periods = periods;
		
		// HACK: snd_pcm_delay seems not to work for the dsnoop device.
		//       This code determines if snd_pcm is working. As the capture
		//       device just started, it should return zero or a small number.
		//       So if it returns that more than half of the buffer is filled
		//       already, it seems broken.
		rec_delay_broken = false;
		if (get_buffer_space(true) > rec_buffersize / 2) {
			rec_delay_broken = true;
			log_file->write_report(
				"snd_pcm_delay does not work for capture buffer.",
				"t_alsa_io::open", LOG_NORMAL, LOG_DEBUG);
		}
		
		log_file->write_header("t_alsa_io::open", LOG_NORMAL, LOG_DEBUG);
		log_file->write_raw("ALSA capture buffer settings.\n");
		log_file->write_raw("Rate = ");
		log_file->write_raw(card_rate);
		log_file->write_raw(" frames/sec\n");
		log_file->write_raw("Frame size = ");
		log_file->write_raw(rec_framesize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Periods = ");
		log_file->write_raw(rec_periods);
		log_file->write_endl();
		log_file->write_raw("Period size = ");
		log_file->write_raw(buffersize * rec_framesize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Buffer size = ");
		log_file->write_raw(rec_buffersize);
		log_file->write_raw(" bytes\n");
		log_file->write_raw("Can pause: ");
		log_file->write_bool(can_pause);
		log_file->write_endl();
		log_file->write_footer();
	}
	
	return true;
#undef HANDLE_ALSA_ERROR
}


void t_alsa_io::enable(bool enable_playback, bool enable_recording) {
	if (!can_pause) return;

	if (pcm_play_ptr) {
		snd_pcm_pause(pcm_play_ptr, (int)enable_playback);
	}
	if (pcm_rec_ptr) {
		snd_pcm_pause(pcm_rec_ptr, (int)enable_recording);
	}
}

void t_alsa_io::flush(bool playback_buffer, bool recording_buffer) {
	if (playback_buffer && pcm_play_ptr) {
		// snd_pcm_reset(pcm_play_ptr);
		snd_pcm_drop(pcm_play_ptr);
		snd_pcm_prepare(pcm_play_ptr);
		snd_pcm_start(pcm_play_ptr);
	}
	if (recording_buffer && pcm_rec_ptr) {
		// For some obscure reason snd_pcm_reset causes the CPU
		// load to rise to 99.9% when playing and capturing is
		// done on the same sound card.
		// Therefor snd_pcm_reset is replaced by functions to
		// stop the card, drop samples and start again.
		// snd_pcm_reset(pcm_rec_ptr);
		snd_pcm_drop(pcm_rec_ptr);
		snd_pcm_prepare(pcm_rec_ptr);
		snd_pcm_start(pcm_rec_ptr);
	}
}

int t_alsa_io::get_buffer_space(bool is_recording_buffer) {
	int rv;
	int err;
	snd_pcm_sframes_t delay;
	snd_pcm_status_t* status;
	snd_pcm_status_alloca(&status);
	
	if(is_recording_buffer) {
		if(!pcm_rec_ptr) return 0;
		
		// This does not work as snd_pcm_status_get_avail_max does not return
		// accurate results.
		// snd_pcm_status(pcm_rec_ptr, status);
		// rv = rec_framesize * snd_pcm_status_get_avail_max(status);
		
		snd_pcm_hwsync(pcm_rec_ptr);
		if ((err = snd_pcm_delay(pcm_rec_ptr, &delay)) < 0) {
			string msg = "snd_pcm_delay for capture buffer failed: ";
			msg += snd_strerror(err);
			log_file->write_report(msg, "t_alsa_io::get_buffer_space", 
				LOG_NORMAL, LOG_DEBUG);
			delay = 0;
			snd_pcm_prepare(pcm_rec_ptr);
		}

		if (rec_delay_broken) {
			rv = 0; // there is no way to get an accurate number
		} else {
			rv = int(delay * rec_framesize);
		}
		
		if (rv > rec_buffersize) {
			rv = rec_buffersize; // capture buffer overrun
			snd_pcm_prepare(pcm_rec_ptr);
		}
	} else {
		if(!pcm_play_ptr) return 0;
		
		snd_pcm_status(pcm_play_ptr, status);
		rv = play_framesize * snd_pcm_status_get_avail_max(status);
		
		if (rv > play_buffersize) {
			rv = play_buffersize; // playback buffer underrun
			snd_pcm_prepare(pcm_play_ptr);
		}
	}
	
	return rv;
}

int t_alsa_io::get_buffer_size(bool is_recording_buffer)
{
	if (is_recording_buffer) return rec_buffersize;
	else return play_buffersize;
}

bool t_alsa_io::play_buffer_underrun(void) {
	if (!pcm_play_ptr) return false;
	return snd_pcm_state(pcm_play_ptr) == SND_PCM_STATE_XRUN;
}

int t_alsa_io::read(unsigned char* buf, int len) {
	string msg;
	
	if (!pcm_rec_ptr) {
		log_file->write_report("Illegal pcm_rec_prt.", 
			"t_alsa_io::read", LOG_NORMAL, LOG_CRITICAL);
		return -1;
	}
	
	int len_frames = len / rec_framesize;
	int read_frames = 0;
	
	for(;;) {
		int read = snd_pcm_readi(pcm_rec_ptr, buf, len_frames);
		if (read == -EPIPE) {
			msg = "Capture buffer overrun.";
			log_file->write_report(msg, "t_alsa_io::read", LOG_NORMAL, LOG_DEBUG);
			snd_pcm_prepare(pcm_rec_ptr);
			snd_pcm_start(pcm_rec_ptr);
			continue;
		} else if (read <= 0) {
			msg = "PCM read error: ";
			msg += snd_strerror(read);
			log_file->write_report(msg, "t_alsa_io::read", LOG_NORMAL, LOG_DEBUG);
			return -1;
		} else if (read < len_frames) {
			buf += rec_framesize * read;
			len_frames -= read;
			read_frames += read;
			continue;
		}
		return (read_frames + read) * rec_framesize;
	}
}
int t_alsa_io::write(const unsigned char* buf, int len) {	
	int len_frames = len / play_framesize;
	int frames_written = 0;
	string msg;
	
	for (;;) {
		if(!pcm_play_ptr) return -1;
		int written = snd_pcm_writei(pcm_play_ptr, buf, len_frames);
		if (written == -EPIPE) {
			msg = "Playback buffer underrun.";
			log_file->write_report(msg, "t_alsa_io::write", LOG_NORMAL, LOG_DEBUG);
			snd_pcm_prepare(pcm_play_ptr);
			continue;
		} else if (written == -EINVAL) {
			msg = "Invalid argument passed to snd_pcm_writei: ";
			msg += snd_strerror(written);
			log_file->write_report(msg, "t_alsa_io::write", LOG_NORMAL, LOG_DEBUG);
		} else if (written < 0) {
			msg = "PCM write error: ";
			msg += snd_strerror(written);
			log_file->write_report(msg, "t_alsa_io::write", LOG_NORMAL, LOG_DEBUG);
			return -1;
		} else if (written < len_frames) {
			buf += written * play_framesize;
			len_frames -= written;
			frames_written += written;
			continue;
		}
		return (frames_written + written) * play_framesize;
	}
}


// This function fills the specified list with ALSA hardware soundcards found on the system.
// It uses plughw:xx instead of hw:xx for specifiers, because hw:xx are not practical to
// use (e.g. they require a resampler/channel mixer in the application).
// playback indicates if a list with playback or capture devices should be created.
void alsa_fill_soundcards(list<t_audio_device>& l, bool playback)
{
	int err = 0;
	int card = -1, device = -1;
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);

	for (;;) {
		err = snd_card_next(&card);
		if (err < 0 || card < 0) break;
		if (card >= 0) {
			string name = "hw:";
			name += int2str(card);
			if ((err = snd_ctl_open(&handle, name.c_str(), 0)) < 0) continue;
			if ((err = snd_ctl_card_info(handle, info)) < 0) {
				snd_ctl_close(handle);
				continue;
			}
			
			const char *card_name = snd_ctl_card_info_get_name(info);
			
			for (;;) {
				err = snd_ctl_pcm_next_device(handle, &device);
				if (err < 0 || device < 0) break;

				snd_pcm_info_set_device(pcminfo, device);
				snd_pcm_info_set_subdevice(pcminfo, 0);
				
				if (playback) {
					snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
				} else {
					snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
				}
				
				if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) continue;

				t_audio_device dev;
				dev.device = string("plughw:") + int2str(card) + 
					string(",") + int2str(device);
				dev.name = string(card_name) + " (";
				dev.name += snd_pcm_info_get_name(pcminfo);
				dev.name += ")";
				dev.type = t_audio_device::ALSA;
				l.push_back(dev);
				
			}
			
			snd_ctl_close(handle);
		}
	}
}

// endif ifdef HAVE_LIBASOUND
#endif
