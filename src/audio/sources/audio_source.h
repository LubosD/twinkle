#ifndef AUDIO_SOURCE_H
#define AUDIO_SOURCE_H
#include <stdint.h>
#include <stddef.h>

class t_audio_source
{
public:
	virtual ~t_audio_source() {}

	// The audio is always signed 16-bit, 44100 Hz, mono
	virtual size_t get_audio_samples(uint8_t* buf, size_t buf_size) = 0;
};

#endif // AUDIO_SOURCE_H

