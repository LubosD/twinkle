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

#ifndef _TONE_GEN_H
#define _TONE_GEN_H

#include <string>
#include <fstream>
#include <cc++/config.h>
#include <sndfile.h>
#include "sys_settings.h"
#include "threads/mutex.h"
#include "threads/thread.h"
#include "threads/sema.h"

#ifndef _AUDIO_DEVICE_H
class t_audio_io;
#endif

using namespace std;

class t_tone_gen {
private:
	string		wav_filename;	// name of wav file
	SNDFILE		*wav_file;	// SNDFILE pointer to wav file
	SF_INFO 	wav_info;	// Information about format of the wav file
	t_audio_device	dev_tone;	// device to play tone
	t_audio_io*	aio;		// soundcard
	bool		valid;		// wav file is in a valid format
	bool		stop_playing;	// indicates if playing should stop
	t_thread	*thr_play;	// playing thread
	bool		loop;		// repeat playing
	int		pause;		// pause (ms) between repetitions
	short		*data_buf;	// buffer for reading sound samples
	t_semaphore	sema_finished;	// indicates if playing finished

public:
	t_tone_gen(const string &filename, const t_audio_device &_dev_tone);
	~t_tone_gen();

	bool is_valid(void) const;

	// Play the wav file
	// loop = true -> repeat playing
	// pause is pause in ms between repetitions
	void start_play_thread(bool _loop, int _pause);
	void play(void);

	// Stop playing
	void stop(void);
};

#endif
