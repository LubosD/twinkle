AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src \
	$(CCRTP_CFLAGS) \
	$(XML2_CFLAGS)

noinst_LIBRARIES = libaudio.a

libaudio_a_SOURCES =\
	audio_device.cpp\
	audio_decoder.cpp\
	audio_encoder.cpp\
        audio_codecs.cpp\
        audio_rx.cpp\
        audio_session.cpp\
        audio_tx.cpp\
        dtmf_player.cpp\
        freq_gen.cpp\
        g711.cpp\
        g721.cpp\
	g723_16.cpp\
        g723_24.cpp\
        g723_40.cpp\
        g72x.cpp\
        media_buffer.cpp\
        rtp_telephone_event.cpp\
        tone_gen.cpp\
        twinkle_rtp_session.cpp\
        twinkle_zrtp_ui.cpp\
        audio_device.h\
	audio_decoder.h\
        audio_encoder.h\
        audio_codecs.h\
        audio_rx.h\
        audio_session.h\
        audio_tx.h\
        dtmf_player.h\
        freq_gen.h\
        g711.h\
        g72x.h\
        media_buffer.h\
        rtp_telephone_event.h\
        tone_gen.h\
        twinkle_rtp_session.h\
        twinkle_zrtp_ui.h

SUBDIRS = gsm

EXTRA_DIST = README_G711
