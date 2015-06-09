#include "audio_source_rtp.h"
#include <cstring>
#include <memory>
#include <limits>
#include <iostream>

#define SAMPLE_BUF_SIZE (MAX_PTIME * 44100/1000)

t_audio_source_rtp::t_audio_source_rtp(t_twinkle_rtp_session* rtp, t_user *user_config,
									   const std::map<unsigned short, t_audio_codec> &payload2codec, t_audio_codec codec,
									   unsigned short ptime)
	: m_rtp_session(rtp), m_user_config(user_config), m_payload2codec(payload2codec), m_codec(codec), m_ptime(ptime)
{
	::gettimeofday(&m_jitter_depleted_time, nullptr);

	m_map_audio_decoder[CODEC_G711_ALAW] = new t_g711a_audio_decoder(m_ptime, user_config);

	m_map_audio_decoder[CODEC_G711_ULAW] = new t_g711u_audio_decoder(m_ptime, user_config);

	m_map_audio_decoder[CODEC_GSM] = new t_gsm_audio_decoder(user_config);

#ifdef HAVE_SPEEX
	m_map_audio_decoder[CODEC_SPEEX_NB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_NB, user_config);

	m_map_audio_decoder[CODEC_SPEEX_WB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_WB, user_config);

	m_map_audio_decoder[CODEC_SPEEX_UWB] = new t_speex_audio_decoder(
			t_speex_audio_decoder::MODE_UWB, user_config);
#endif
#ifdef HAVE_ILBC
	m_map_audio_decoder[CODEC_ILBC] = new t_ilbc_audio_decoder(m_ptime, user_config);
#endif

	m_map_audio_decoder[CODEC_G726_16] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_16, m_ptime, user_config);

	m_map_audio_decoder[CODEC_G726_24] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_24, m_ptime, user_config);

	m_map_audio_decoder[CODEC_G726_32] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_32, m_ptime, user_config);

	m_map_audio_decoder[CODEC_G726_40] = new t_g726_audio_decoder(
			t_g726_audio_decoder::BIT_RATE_40, m_ptime, user_config);

	int error;
	m_resampler = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);

	m_decBuf.reset(new int16_t[SAMPLE_BUF_SIZE]);
	m_resampleIn.reset(new float[SAMPLE_BUF_SIZE]);
	m_resampleOut.reset(new float[SAMPLE_BUF_SIZE]);
}

t_audio_source_rtp::~t_audio_source_rtp()
{
	for (auto it = m_map_audio_decoder.begin(); it != m_map_audio_decoder.end(); it++)
		delete it->second;

	src_delete(m_resampler);
}

bool t_audio_source_rtp::keepFillingJitterBuffer(struct timeval& now)
{
	/*
	long msdiff;

	if (now.tv_sec >= m_jitter_depleted_time.tv_sec+2)
		return false;

	msdiff = (now.tv_sec - m_jitter_depleted_time.tv_sec) * 1000;
	msdiff += (int(now.tv_usec) - int(m_jitter_depleted_time.tv_usec)) / 1000;

	// std::cout << "msdiff is " << msdiff;

	return msdiff < JITTER_BUFFER_SIZE;
	*/

	uint32_t first = m_rtp_session->getFirstTimestamp();
	uint32_t last = m_rtp_session->getLastTimestamp();
	int samplesForJitter = JITTER_BUFFER_SIZE*audio_sample_rate(m_codec)/1000;

	std::cout << (last-first) << " samples in jitter buffer\n";

	return (last - first) < samplesForJitter;
}

size_t t_audio_source_rtp::get_audio_samples(uint8_t* buf, size_t buf_size)
{
	std::unique_ptr<const ost::AppDataUnit> adu;
	struct timeval now;
	size_t rv;

	::gettimeofday(&now, nullptr);

	if (m_jitter && keepFillingJitterBuffer(now))
	{
		// Call us later, we're still filling up.
		// Lower layers will fill the buffer with silence.
		std::cout << "Jitter still filling up at " << now.tv_sec << ";" << now.tv_usec << std::endl;
		return 0;
	}
	else
		m_jitter = false;

	if (rv = flushDecBuf(buf, buf_size))
		return rv;

	do
	{
		if (m_pending_adu)
			adu.reset(m_pending_adu.release());
		else
			adu.reset(m_rtp_session->getData(m_rtp_session->getFirstTimestamp()));

		if (!adu || adu->getSize() == 0)
		{
			// We've run out of data in jitter buffer.
			// Start buffering again.
			m_jitter_depleted_time = now;
			m_jitter = true;
			std::cout << "No RTP data at " << now.tv_sec << ";" << now.tv_usec << std::endl;
			return 0;
		}

		rv = processAdu(adu, buf, buf_size);
	}
	while (rv == 0);

	return rv;
}

size_t t_audio_source_rtp::flushDecBuf(uint8_t* buf, size_t buf_size)
{
	if (m_decBufCount == 0)
		return 0;

	size_t toCopy = std::min<size_t>(buf_size, m_decBufCount*2);
	std::memcpy(buf, &m_decBuf[m_decBufOffset], toCopy);

	m_decBufCount -= toCopy/2;

	std::cout << "flushDecBuf: " << toCopy << " bytes\n";
	return toCopy;
}

size_t t_audio_source_rtp::processAdu(std::unique_ptr<const ost::AppDataUnit>& adu, uint8_t* buf, size_t buf_size)
{
	if (m_last_seq != -1 && uint16_t(m_last_seq)+1 != adu->getSeqNum())
	{
		// TODO: We have a lost incoming RTP packet. Conceal it.
		// For now, we insert silence.
		int numLost;

		numLost = (uint16_t(m_last_seq)+1) - adu->getSeqNum();

		std::cout << "Lost " << numLost << " RTP packets\n";

		std::fill(m_decBuf.get(), m_decBuf.get() + m_lastDecSampleCount, 0);
		m_decBufOffset = 0;
		m_decBufCount = m_lastDecSampleCount;

		m_pending_adu.reset(adu.release());

		return flushDecBuf(buf, buf_size);
	}

	// TODO: codec change, incoming DTMF

	// Check validity
	if (!m_map_audio_decoder[m_codec]->valid_payload_size(adu->getSize(), SAMPLE_BUF_SIZE))
		return 0;

	// Decompress audio
	unsigned char *payload = const_cast<uint8 *>(adu->getData());
	size_t dec_samples;

	dec_samples = m_map_audio_decoder[m_codec]->decode(payload, adu->getSize(), m_decBuf.get(), SAMPLE_BUF_SIZE);
	std::cout << "Decoded " << dec_samples << " samples\n";
	if (dec_samples == 0)
		return 0; // failed decoding

	m_lastDecSampleCount = dec_samples;
	m_decBufCount = resampleDecBuf(dec_samples);
	m_decBufOffset = 0;

	return flushDecBuf(buf, buf_size);
}

size_t t_audio_source_rtp::resampleDecBuf(size_t samplesIn)
{
	SRC_DATA sdata;

	s16toFloat(m_decBuf.get(), m_resampleIn.get(), samplesIn);

	sdata.data_in = m_resampleIn.get();
	sdata.input_frames = samplesIn;
	sdata.data_out = m_resampleOut.get();
	sdata.output_frames = SAMPLE_BUF_SIZE;
	sdata.end_of_input = 0;
	sdata.src_ratio = 44100.f / audio_sample_rate(m_codec);

	src_process(m_resampler, &sdata);
	assert(sdata.input_frames_used == samplesIn);

	floatToS16(m_resampleOut.get(), m_decBuf.get(), sdata.output_frames_gen);
	return sdata.output_frames_gen;
}

void t_audio_source_rtp::s16toFloat(const int16_t* in, float* out, size_t frameCount)
{
	for (size_t i = 0; i < frameCount; i++)
		out[i] = float(in[i]) / std::numeric_limits<int16_t>::max();
}

void t_audio_source_rtp::floatToS16(const float* in, int16_t* out, size_t frameCount)
{
	for (size_t i = 0; i < frameCount; i++)
		out[i] = int16_t(float(in[i]) * std::numeric_limits<int16_t>::max());
}
