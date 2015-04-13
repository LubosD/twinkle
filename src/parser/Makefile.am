AM_CPPFLAGS = -Wall $(XML2_CFLAGS) -I$(top_srcdir)/src

# bison flags
AM_YFLAGS = -d

# flex flags
# The -s flag is not used on purpose. As it is better to have
# some unmatched symbols echoed to stdout then a jammed flex
# causing the process to die.
AM_LFLAGS = -i

# The output of bison cannot be compiled with the -O2 flag.
# With the -O2 flag g++ crashes. The -O2 flag is stripped from
# CXXFLAGS by configure.
# rumen:
# Above don't work.
# It is enought to drop -funit-at-a-time set when -O2 flag is set.
# The error is:
# "cc1plus: out of memory allocating 336746144 bytes after a total of 10846208 bytes."
# Note user can specify -O2 in CXXFLAGS at configure time !
# Also note that unit-at-a-time if new for gcc 3.4 !
# See http://lists.debian.org/debian-gcc/2006/07/msg00281.html
# that claim to be fixed in gcc-4.1+ .

AM_CXXFLAGS = -fno-unit-at-a-time


# This target is only for testing the parser in isolation
#noinst_PROGRAMS = sipparse
#
#sipparse_SOURCES = main.cpp
#
#sipparse_LDADD = $(top_builddir)/src/util.o\
#	$(top_builddir)/src/parser/libsipparser.a\
#	$(top_builddir)/src/sdp/libsdpparser.a\
#	$(top_builddir)/src/sockets/libsocket.a\
#	-lresolv

noinst_LIBRARIES = libsipparser.a

libsipparser_a_SOURCES =\
        challenge.cpp\
        coding.cpp\
        credentials.cpp\
        definitions.cpp\
        hdr_accept.cpp\
        hdr_accept_encoding.cpp\
        hdr_accept_language.cpp\
        hdr_alert_info.cpp\
        hdr_allow.cpp\
	hdr_allow_events.cpp\
        hdr_auth_info.cpp\
        hdr_authorization.cpp\
        hdr_call_id.cpp\
        hdr_call_info.cpp\
        hdr_contact.cpp\
        hdr_content_disp.cpp\
        hdr_content_encoding.cpp\
        hdr_content_language.cpp\
        hdr_content_length.cpp\
        hdr_content_type.cpp\
        hdr_cseq.cpp\
        hdr_date.cpp\
        hdr_error_info.cpp\
	hdr_event.cpp\
        hdr_expires.cpp\
        hdr_from.cpp\
        hdr_in_reply_to.cpp\
        hdr_max_forwards.cpp\
        hdr_min_expires.cpp\
        hdr_mime_version.cpp\
        hdr_organization.cpp\
        hdr_priority.cpp\
        hdr_privacy.cpp\
        hdr_p_asserted_identity.cpp\
        hdr_p_preferred_identity.cpp\
        hdr_proxy_authenticate.cpp\
        hdr_proxy_authorization.cpp\
        hdr_proxy_require.cpp\
        hdr_rack.cpp\
        hdr_record_route.cpp\
        hdr_refer_sub.cpp\
	hdr_refer_to.cpp\
	hdr_referred_by.cpp\
	hdr_replaces.cpp\
        hdr_reply_to.cpp\
        hdr_require.cpp\
        hdr_request_disposition.cpp\
        hdr_retry_after.cpp\
        hdr_route.cpp\
        hdr_rseq.cpp\
        hdr_server.cpp\
        hdr_service_route.cpp\
        hdr_sip_etag.cpp\
        hdr_sip_if_match.cpp\
        hdr_subject.cpp\
	hdr_subscription_state.cpp\
        hdr_supported.cpp\
        hdr_timestamp.cpp\
        hdr_to.cpp\
        hdr_unsupported.cpp\
        hdr_user_agent.cpp\
        hdr_via.cpp\
        hdr_warning.cpp\
        hdr_www_authenticate.cpp\
        header.cpp\
        identity.cpp\
        media_type.cpp\
        milenage.cpp\
        parameter.cpp\
        parse_ctrl.cpp\
	parser.yxx\
        request.cpp\
        response.cpp\
        rijndael.cpp\
        route.cpp\
	scanner.lxx\
        sip_body.cpp\
        sip_message.cpp\
        challenge.h\
        coding.h\
        credentials.h\
        definitions.h\
        hdr_accept.h\
        hdr_accept_encoding.h\
        hdr_accept_language.h\
        hdr_alert_info.h\
        hdr_allow.h\
	hdr_allow_events.h\
        hdr_auth_info.h\
        hdr_authorization.h\
        hdr_call_id.h\
        hdr_call_info.h\
        hdr_contact.h\
        hdr_content_disp.h\
        hdr_content_encoding.h\
        hdr_content_language.h\
        hdr_content_length.h\
        hdr_content_type.h\
        hdr_cseq.h\
        hdr_date.h\
        hdr_error_info.h\
	hdr_event.h\
        hdr_expires.h\
        hdr_from.h\
        hdr_in_reply_to.h\
        hdr_max_forwards.h\
        hdr_min_expires.h\
        hdr_mime_version.h\
        hdr_organization.h\
        hdr_p_asserted_identity.h\
        hdr_p_preferred_identity.h\
        hdr_priority.h\
        hdr_privacy.h\
        hdr_proxy_authenticate.h\
        hdr_proxy_authorization.h\
        hdr_proxy_require.h\
        hdr_rack.h\
        hdr_record_route.h\
        hdr_refer_sub.h\
	hdr_refer_to.h\
	hdr_referred_by.h\
	hdr_replaces.h\
        hdr_reply_to.h\
        hdr_require.h\
        hdr_request_disposition.h\
        hdr_retry_after.h\
        hdr_route.h\
        hdr_rseq.h\
        hdr_server.h\
        hdr_service_route.h\
        hdr_sip_etag.h\
        hdr_sip_if_match.h\
        hdr_subject.h\
	hdr_subscription_state.h\
        hdr_supported.h\
        hdr_timestamp.h\
        hdr_to.h\
        hdr_unsupported.h\
        hdr_user_agent.h\
        hdr_via.h\
        hdr_warning.h\
        hdr_www_authenticate.h\
        header.h\
        identity.h\
        media_type.h\
        milenage.h\
        parameter.h\
        parse_ctrl.h\
        request.h\
        response.h\
        rijndael.h\
        route.h\
        sip_body.h\
        sip_message.h
