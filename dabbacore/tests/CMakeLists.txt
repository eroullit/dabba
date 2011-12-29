INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include)
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/${PROJECT_NAME})

ADD_EXECUTABLE(test_packet_mmap test_packet_mmap.c)
TARGET_LINK_LIBRARIES (test_packet_mmap ${PROJECT_NAME})
ADD_TEST(test_packet_mmap test_packet_mmap)
