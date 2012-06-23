INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include)
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/${PROJECT_NAME})

#ADD_EXECUTABLE(test_packet_mmap test_packet_mmap.c)
#TARGET_LINK_LIBRARIES (test_packet_mmap ${PROJECT_NAME})
#ADD_TEST(test_packet_mmap test_packet_mmap)

ADD_EXECUTABLE(test_strlcpy test_strlcpy.c)
TARGET_LINK_LIBRARIES (test_strlcpy ${PROJECT_NAME})
ADD_TEST(test_strlcpy test_strlcpy)

ADD_EXECUTABLE(test_pcap test_pcap.c)
TARGET_LINK_LIBRARIES (test_pcap ${PROJECT_NAME})
ADD_TEST(test_pcap test_pcap)

#ADD_CUSTOM_TARGET(setcap-test_packet_mmap COMMAND ${SETCAP_EXECUTABLE} cap_net_raw,cap_ipc_lock,cap_net_admin=eip test_packet_mmap)
#ADD_DEPENDENCIES(setcap setcap-test_packet_mmap)
