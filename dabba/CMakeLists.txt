PROJECT(dabba C)

FIND_PACKAGE(Doxygen)
INCLUDE(Pod2Man)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libdabba/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libdabba-rpc/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/dabbad/include)

# FIXME Fix path when rpc headers are generated in the right include directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	ADD_CUSTOM_TARGET(${PROJECT_NAME}-doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_DEPENDENCIES(doc ${PROJECT_NAME}-doc)
ENDIF(DOXYGEN_FOUND)

ADD_EXECUTABLE(${PROJECT_NAME}
	dabba.c
	help.c
	cli.c
	interface.c
	interface-status.c
	interface-settings.c
	interface-driver.c
	interface-capabilities.c
	interface-coalesce.c
	interface-pause.c
	interface-offload.c
	interface-statistics.c
	rpc.c
	capture.c
	capture-settings.c
	thread.c
	thread-capabilities.c
	thread-settings.c
)

TARGET_LINK_LIBRARIES (${PROJECT_NAME} libdabba-rpc)

POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/dabba.c dabba 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/interface.c dabba-interface 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/capture.c dabba-capture 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/thread.c dabba-thread 1)

INSTALL(TARGETS dabba DESTINATION sbin COMPONENT applications)

ADD_SUBDIRECTORY(test)
