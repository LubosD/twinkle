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

#ifndef _AUDIO_DEVICE_H
#define _AUDIO_DEVICE_H

#include <string>
#include "twinkle_config.h"

using namespace std;

#ifndef _SYS_SETTINGS_H
class t_audio_device;
#endif

enum t_audio_sampleformat {
	SAMPLEFORMAT_U8,
	SAMPLEFORMAT_S8,
	SAMPLEFORMAT_S16,
	SAMPLEFORMAT_U16
};

class t_audio_io {
public:
	virtual ~t_audio_io();
	virtual void enable(bool enable_playback, bool enable_recording) = 0;
	virtual void flush(bool playback_buffer, bool recording_buffer) = 0;
	// Returns the number of bytes that can be written/read without blocking
	virtual int get_buffer_space(bool is_recording_buffer) = 0;
	// Returns the size of the hardware buffer
	virtual int get_buffer_size(bool is_recording_buffer) = 0;
	
	/** Check if a play buffer underrun occurred. */
	virtual bool play_buffer_underrun(void) = 0;
	
	virtual int read(unsigned char* buf, int len) = 0;
	virtual int write(const unsigned char* buf, int len) = 0;
	virtual int get_sample_rate(void) const;
	
	static t_audio_io* open(const t_audio_device& dev, bool playback, 
		bool capture, bool blocking, int channels, t_audio_sampleformat format, 
		int sample_rate, bool short_latency);
		
	// Validate if an audio device can be opened.
	static bool validate(const t_audio_device& dev, bool playback, bool capture);

protected:
	virtual bool open(const string& device, bool playback, bool capture, 
		bool blocking, int channels, t_audio_sampleformat format, 
		int sample_rate, bool short_latency);
		
private:
	int _sample_rate;
	
};

class t_oss_io : public t_audio_io {
public:
	t_oss_io();
	virtual ~t_oss_io();
	void enable(bool enable_playback, bool enable_recording);
	void flush(bool playback_buffer, bool recording_buffer);
	int get_buffer_space(bool is_recording_buffer);
	int get_buffer_size(bool is_recording_buffer);
	bool play_buffer_underrun(void);
	int read(unsigned char* buf, int len);
	int write(const unsigned char* buf, int len);
protected:
	bool open(const string& device, bool playback, bool capture, bool blocking, 
		int channels, t_audio_sampleformat format, int sample_rate, 
		bool short_latency);
private:
	int fd;
	int play_buffersize, rec_buffersize;
};

#ifdef HAVE_LIBASOUND
class t_alsa_io : public t_audio_io {
public:
	t_alsa_io();
	virtual ~t_alsa_io();
	void enable(bool enable_playback, bool enable_recording);
	void flush(bool playback_buffer, bool recording_buffer);
	int get_buffer_space(bool is_recording_buffer);
	int get_buffer_size(bool is_recording_buffer);
	bool play_buffer_underrun(void);
	int read(unsigned char* buf, int len);
	int write(const unsigned char* buf, int len);
protected:
	bool open(const string& device, bool playback, bool capture, bool blocking, 
		int channels, t_audio_sampleformat format, int sample_rate, 
		bool short_latency);
private:
	struct _snd_pcm *pcm_play_ptr, *pcm_rec_ptr;
	int play_framesize, rec_framesize;
	int play_buffersize, rec_buffersize;
	int play_periods, rec_periods;
	bool short_latency;
	
	// snd_pcm_delay should return the number of bytes in the buffer.
	// For some reason however, if the capture device is a software mixer,
	// it returns inaccurate values.
	// This flag if the functionality is broken.
	bool rec_delay_broken;
	
	// Indicates if snd_pcm_pause works for this device
	bool can_pause;
};
#endif

#endif
