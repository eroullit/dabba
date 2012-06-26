PROJECT(dabba C)

# States that CMake required version must be >= 2.6
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
# Activate this when you want to build a shared lib out the the lib/ directory
# Bear in mind that no profiling will be available on 64-bit arch when on
# SET(BUILD_SHARED_LIBS ON)

FIND_PACKAGE(Threads REQUIRED)

IF(CMAKE_HAVE_PTHREAD_CREATE)
	SET(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
ELSE(CMAKE_HAVE_PTHREAD_CREATE)
	MESSAGE(FATAL_ERROR "The framework only supports pthread")
ENDIF(CMAKE_HAVE_PTHREAD_CREATE)

INCLUDE(CheckBPFAttach)
INCLUDE(CheckPFPacket)
INCLUDE(CheckStrictAlign)
INCLUDE(CheckTxRing)

ADD_DEFINITIONS(
	-W
	-Wextra
	-Wall
	-Werror
	-std=gnu99
	-O2
	-D_REENTRANT
	-D_FORTIFY_SOURCE=2
	-fstack-protector
)

IF(CMAKE_C_COMPILER_ID STREQUAL GNU)
	ADD_DEFINITIONS(
		-pedantic
		-pedantic-errors
		-z relo
		-pie
	)
ENDIF(CMAKE_C_COMPILER_ID STREQUAL GNU)

IF (CMAKE_BUILD_TYPE STREQUAL profile)
	ADD_DEFINITIONS(-g -pg)
ENDIF (CMAKE_BUILD_TYPE STREQUAL profile)

ENABLE_TESTING()

ADD_CUSTOM_TARGET(setcap)
ADD_CUSTOM_TARGET(doc)

ADD_SUBDIRECTORY(libdabba)
ADD_SUBDIRECTORY(dabbad)
ADD_SUBDIRECTORY(dabba)
