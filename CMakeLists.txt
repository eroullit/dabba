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

SET(CMAKE_INSTALL_PREFIX "/usr")

INCLUDE(FindDebArch)

EXECUTE_PROCESS(COMMAND "${CMAKE_SOURCE_DIR}/debian/fixperms.sh")

SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_GENERATOR "TGZ;DEB;RPM")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CPACK_RESOURCE_FILE_README}")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
#SET(CPACK_PACKAGE_VERSION_PATCH "1")
#SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_orig")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libcap2-bin, libprotobuf-c0, libnl-3-200, libnl-genl-3-200, libnl-route-3-200, adduser")
SET(CPACK_PACKAGE_VENDOR "Emmanuel Roullit")
SET(CPACK_PACKAGE_CONTACT "${CPACK_PACKAGE_VENDOR} <emmanuel.roullit@gmail.com>")
SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/debian/postinst;${CMAKE_SOURCE_DIR}/debian/postrm;${CMAKE_SOURCE_DIR}/debian/shlibs;${CMAKE_SOURCE_DIR}/debian/conffiles")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION
"Suite of low-level multithreaded zero-copy network tools.
 dabba is based on 4 parts:
  - libdabba - low-level zero-copy network library
  - libdabba-rpc - protobuf-based rpc library for dabbad and dabba
  - dabbad - multi-threaded task manager and IPC query processor
  - dabba - CLI 'à la git' to communicate with dabbad and submit tasks"
)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Suite of low-level multithreaded zero-copy network tools.")
SET(CPACK_PACKAGE_DESCRIPTION "dabba is a set of multithreaded network tools using zero-copy features.")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "net")
SET(CPACK_STRIP_FILES ON)
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.gitignore;/*.swp;~$;${CPACK_SOURCE_IGNORE_FILES}")

INCLUDE(CPack)
INCLUDE(CheckBPFAttach)
INCLUDE(CheckPFPacket)
INCLUDE(CheckStrictAlign)
INCLUDE(CheckTxRing)
INCLUDE(Pod2Man)

SET(CMAKE_C_FLAGS_RELEASE "")

ADD_DEFINITIONS(
	-W
	-Wextra
	-Wall
	-Werror
	-std=gnu99
	-O2
	-D_REENTRANT
	-D_FORTIFY_SOURCE=2
	-D_FILE_OFFSET_BITS=64
	-DDABBA_VERSION="${CPACK_PACKAGE_VERSION}"
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
ADD_MANPAGE_TARGET()

ADD_SUBDIRECTORY(libdabba)
ADD_SUBDIRECTORY(libdabba-rpc)
ADD_SUBDIRECTORY(dabbad)
ADD_SUBDIRECTORY(dabba)

# HACK
INSTALL(FILES debian/copyright debian/changelog.gz DESTINATION share/doc/${PROJECT_NAME})
