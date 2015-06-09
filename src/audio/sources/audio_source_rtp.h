#ifndef T_AUDIO_SOURCE_RTP_H
#define T_AUDIO_SOURCE_RTP_H
#include "audio_source.h"
#include "../twinkle_rtp_session.h"
#include "../audio_codecs.h"
#include "../audio_decoder.h"
#include <sys/time.h>
#include <map>
#include <samplerate.h>

class t_audio_source_rtp : public t_audio_source
{
public:
	t_audio_source_rtp(t_twinkle_rtp_session* rtp, t_user *user_config,
					   const std::map<unsigned short, t_audio_codec> &payload2codec,
					   t_audio_codec codec, unsigned short ptime);
	virtual ~t_audio_source_rtp();

	virtual size_t get_audio_samples(uint8_t* buf, size_t buf_size) override;
private:
	// Wherther the code should keep on stalling output
	bool keepFillingJitterBuffer(struct timeval& now);

	// Processes an input RTP data unit
	size_t processAdu(std::unique_ptr<const ost::AppDataUnit>& adu, uint8_t* buf, size_t buf_size);

	// Pushes out as much data from m_decBuf as possible
	size_t flushDecBuf(uint8_t* buf, size_t buf_size);

	// Resamples data in m_decBuf, converting from input sample rate to soundcard sample rate
	size_t resampleDecBuf(size_t samplesIn);

	// Conversions between S16 and float, needed for libsamplerate (only accepts float)
	static void s16toFloat(const int16_t* in, float* out, size_t frameCount);
	static void floatToS16(const float* in, int16_t* out, size_t frameCount);
private:
	t_twinkle_rtp_session* m_rtp_session;
	std::map<t_audio_codec, t_audio_decoder *> m_map_audio_decoder;
	t_user *m_user_config;
	const std::map<unsigned short, t_audio_codec>& m_payload2codec;
	t_audio_codec m_codec;
	unsigned short m_ptime;

	// We manage the jitter buffer by retrieving RTP data from ccrtp with a delay.
	struct timeval m_jitter_depleted_time;

	// Pending ADU is an ADU we received after a packet loss
	std::unique_ptr<const ost::AppDataUnit> m_pending_adu;

	std::unique_ptr<int16_t[]> m_decBuf;
	size_t m_decBufOffset, m_decBufCount = 0;
	size_t m_lastDecSampleCount;
	std::unique_ptr<float[]> m_resampleIn, m_resampleOut;

	// Last seq num from RTP stream
	int m_last_seq = -1;

	// Sound card sample rate
	int m_sc_sample_rate;
	SRC_STATE* m_resampler;

	bool m_jitter = false;

	static const int JITTER_BUFFER_SIZE = 100; // 100ms
};

#endif // T_AUDIO_SOURCE_RTP_H
