project(libtwinkle-audio)

set(LIBTWINKLE_AUDIO-SRCS
	audio_device.cpp
	audio_decoder.cpp
	audio_encoder.cpp
	audio_codecs.cpp
	audio_rx.cpp
	audio_session.cpp
	audio_tx.cpp
	dtmf_player.cpp
	freq_gen.cpp
	g711.cpp
	g721.cpp
	g723_16.cpp
	g723_24.cpp
	g723_40.cpp
	g72x.cpp
	media_buffer.cpp
	rtp_telephone_event.cpp
	tone_gen.cpp
	twinkle_rtp_session.cpp
	twinkle_zrtp_ui.cpp
)

if(HAVE_G722)
	list(APPEND LIBTWINKLE_AUDIO-SRCS
		g722_decode.c
		g722_encode.c
	)
endif()

add_library(libtwinkle-audio OBJECT ${LIBTWINKLE_AUDIO-SRCS})
