PROJECT(dabba C)

FIND_PACKAGE(Doxygen)
INCLUDE(Pod2Man)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libdabba/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/dabbad/include)

IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	ADD_CUSTOM_TARGET(${PROJECT_NAME}-doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	ADD_DEPENDENCIES(doc ${PROJECT_NAME}-doc)
ENDIF(DOXYGEN_FOUND)

ADD_EXECUTABLE(${PROJECT_NAME} dabba.c help.c interface.c interface-settings.c interface-driver.c interface-capabilities.c interface-coalesce.c interface-pause.c interface-offload.c ipc.c capture.c thread.c)

POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/dabba.c dabba 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/interface.c dabba-interface 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/capture.c dabba-capture 1)
POD2MAN(${CMAKE_CURRENT_SOURCE_DIR}/thread.c dabba-thread 1)

INSTALL(TARGETS dabba DESTINATION sbin COMPONENT applications)

ADD_SUBDIRECTORY(test)
