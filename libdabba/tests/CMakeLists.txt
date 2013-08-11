INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include)
LINK_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})

FOREACH(COMP test-packet-mmap test-pcap)
	ADD_EXECUTABLE(${COMP} ${COMP}.c)
	TARGET_LINK_LIBRARIES (${COMP} ${PROJECT_NAME})
	ADD_TEST(${COMP} ${COMP})
ENDFOREACH(COMP)

ADD_CUSTOM_TARGET(test-packet-mmap-setcap COMMAND ${SETCAP_EXECUTABLE} cap_net_raw,cap_ipc_lock,cap_net_admin=eip test-packet-mmap)
ADD_DEPENDENCIES(setcap test-packet-mmap-setcap)
