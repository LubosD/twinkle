project(libtwinkle)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(audio)
add_subdirectory(audio/gsm/src)
add_subdirectory(audits)
add_subdirectory(im)
add_subdirectory(mwi)
add_subdirectory(parser)
add_subdirectory(patterns)
add_subdirectory(presence)
add_subdirectory(sdp)
add_subdirectory(sockets)
add_subdirectory(stun)
add_subdirectory(threads)
add_subdirectory(utils)

set(LIBTWINKLE_SRCS
	abstract_dialog.cpp
	address_book.cpp
	auth.cpp
	call_history.cpp
	call_script.cpp
	client_request.cpp
	cmd_socket.cpp
	dialog.cpp
	diamondcard.cpp
	epa.cpp
	events.cpp
	id_object.cpp
	line.cpp
	listener.cpp
	log.cpp
	phone.cpp
	phone_user.cpp
	prohibit_thread.cpp
	redirect.cpp
	sender.cpp
	service.cpp
	session.cpp
	sub_refer.cpp
	subscription.cpp
	subscription_dialog.cpp
	sys_settings.cpp
	timekeeper.cpp
	transaction.cpp
	transaction_layer.cpp
	transaction_mgr.cpp
	user.cpp
	userintf.cpp
	util.cpp
)

add_library(libtwinkle OBJECT ${LIBTWINKLE_SRCS})

add_executable(twinkle-console
	main.cpp
	$<TARGET_OBJECTS:libtwinkle>
	$<TARGET_OBJECTS:libtwinkle-audio>
	$<TARGET_OBJECTS:libtwinkle-gsm>
	$<TARGET_OBJECTS:libtwinkle-audits>
	$<TARGET_OBJECTS:libtwinkle-im>
	$<TARGET_OBJECTS:libtwinkle-mwi>
	$<TARGET_OBJECTS:libtwinkle-parser>
	$<TARGET_OBJECTS:libtwinkle-patterns>
	$<TARGET_OBJECTS:libtwinkle-presence>
	$<TARGET_OBJECTS:libtwinkle-sdp>
	$<TARGET_OBJECTS:libtwinkle-sockets>
	$<TARGET_OBJECTS:libtwinkle-stun>
	$<TARGET_OBJECTS:libtwinkle-threads>
	$<TARGET_OBJECTS:libtwinkle-utils>
)

add_subdirectory(gui)

target_link_libraries(twinkle-console -lpthread -lresolv ${LibMagic_LIBRARY} ${LIBXML2_LIBRARIES}
	${Readline_LIBRARY} ${ILBC_LIBRARIES} ${SPEEX_LIBRARIES} ${ZRTPCPP_LIBRARIES}
	${CCRTP_LIBRARIES} ${COMMONCPP_LIBRARIES} ${UCOMMON_LIBRARIES} ${LIBSNDFILE_LIBRARY}
	${Boost_LIBRARIES} ${ALSA_LIBRARY})

install(TARGETS twinkle-console DESTINATION bin)


