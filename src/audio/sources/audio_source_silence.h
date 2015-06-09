#ifndef T_AUDIO_SOURCE_SILENCE_H
#define T_AUDIO_SOURCE_SILENCE_H
#include "audio_source.h"

// Fixes short reads from wrapped source by inserting silence
// or possibly comfort noise.
class t_audio_source_silence : public t_audio_source
{
public:
	t_audio_source_silence(t_audio_source* wrapped);
	virtual ~t_audio_source_silence();

	virtual size_t get_audio_samples(uint8_t* buf, size_t buf_size) override;
private:
	t_audio_source* m_wrapped;
};

#endif // T_AUDIO_SOURCE_SILENCE_H
