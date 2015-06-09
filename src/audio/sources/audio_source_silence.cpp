#include "audio_source_silence.h"
#include <cstring>

t_audio_source_silence::t_audio_source_silence(t_audio_source* wrapped)
	: m_wrapped(wrapped)
{

}

t_audio_source_silence::~t_audio_source_silence()
{
	delete m_wrapped;
}

size_t t_audio_source_silence::get_audio_samples(uint8_t* buf, size_t buf_size)
{
	size_t rd;

	rd = m_wrapped->get_audio_samples(buf, buf_size);

	if (rd == 0)
	{
		if (buf_size < m_lastCount)
			m_lastCount = buf_size;

		std::memset(buf, 0, m_lastCount);
		return m_lastCount;
	}

	m_lastCount = rd;
	return rd;
}
