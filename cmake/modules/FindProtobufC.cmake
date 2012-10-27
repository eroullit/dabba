#
# Copyright (C) 2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
#

# The module defines the following variables:
#  PROTOBUFC_FOUND - true if libprotobufc-c and its headers are found
#  PROTOBUFC_INCLUDE_DIRS - the path to the libprotobufc-c header files
#  PROTOBUFC_LIBRARY - the path to the libprotobufc-c library
# The module defines the following function:
#  PROTOBUFC_GENERATE_C(PROTOSRCS_VAR PROTOHDRS_VAR PROTOFILE) -
# 	function to generate the needed C code from the prototype file.

# Inspired and adapted from CMake FindProtobuf.cmake

FUNCTION(PROTOBUFC_GENERATE_C SRCS HDRS)
  IF(NOT ARGN)
    MESSAGE(SEND_ERROR "ERROR: PROTOBUFC_GENERATE_C() called without any proto files")
    RETURN()
  ENDIF(NOT ARGN)

  IF(PROTOBUFC_GENERATE_C_APPEND_PATH)
    # Create an include path for each file specified
    FOREACH(FIL ${ARGN})
      GET_FILENAME_COMPONENT(ABS_FIL ${FIL} ABSOLUTE)
      GET_FILENAME_COMPONENT(ABS_PATH ${ABS_FIL} PATH)
      LIST(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      IF(${_contains_already} EQUAL -1)
          LIST(APPEND _protobuf_include_path -I ${ABS_PATH})
      ENDIF()
    ENDFOREACH()
  ELSE()
    SET(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  ENDIF()

  SET(${SRCS})
  SET(${HDRS})
  FOREACH(FIL ${ARGN})
    GET_FILENAME_COMPONENT(ABS_FIL ${FIL} ABSOLUTE)
    GET_FILENAME_COMPONENT(FIL_WE ${FIL} NAME_WE)
    
    LIST(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb-c.c")
    LIST(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb-c.h")

    ADD_CUSTOM_COMMAND(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb-c.c"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb-c.h"
      COMMAND  ${PROTOBUFC_PROTOC_EXECUTABLE}
      ARGS --c_out  ${CMAKE_CURRENT_BINARY_DIR} ${_protobuf_include_path} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C protocol buffer compiler on ${FIL}"
      VERBATIM )
  ENDFOREACH()

  SET_SOURCE_FILES_PROPERTIES(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  SET(${SRCS} ${${SRCS}} PARENT_SCOPE)
  SET(${HDRS} ${${HDRS}} PARENT_SCOPE)
ENDFUNCTION()

# Internal function: search for normal library as well as a debug one
#    if the debug one is specified also include debug/optimized keywords
#    in *_LIBRARIES variable
FUNCTION(_protobuf_find_libraries name filename)
   FIND_LIBRARY(${name}_LIBRARY
       NAMES ${filename}
       PATHS ${PROTOBUFC_SRC_ROOT_FOLDER}/vsprojects/Release)
   MARK_AS_ADVANCED(${name}_LIBRARY)

   FIND_LIBRARY(${name}_LIBRARY_DEBUG
       NAMES ${filename}
       PATHS ${PROTOBUFC_SRC_ROOT_FOLDER}/vsprojects/Debug)
   MARK_AS_ADVANCED(${name}_LIBRARY_DEBUG)

   IF(NOT ${name}_LIBRARY_DEBUG)
      # There is no debug library
      SET(${name}_LIBRARY_DEBUG ${${name}_LIBRARY} PARENT_SCOPE)
      SET(${name}_LIBRARIES     ${${name}_LIBRARY} PARENT_SCOPE)
   ELSE()
      # There IS a debug library
      SET(${name}_LIBRARIES
          optimized ${${name}_LIBRARY}
          debug     ${${name}_LIBRARY_DEBUG}
          PARENT_SCOPE
      )
   ENDIF()
ENDFUNCTION()

#
# Main.
#

# By default have PROTOBUFC_GENERATE_C macro pass -I to protoc-c
# for each directory where a proto file is referenced.
IF(NOT DEFINED PROTOBUFC_GENERATE_C_APPEND_PATH)
  SET(PROTOBUFC_GENERATE_C_APPEND_PATH TRUE)
ENDIF()

# The Protobufc library
_PROTOBUF_FIND_LIBRARIES(PROTOBUFC protobuf-c)

# The Protobufc Protoc Library
_PROTOBUF_FIND_LIBRARIES(PROTOBUFC_PROTOC protoc-c)

# Find the include directory
FIND_PATH(PROTOBUFC_INCLUDE_DIR
    google/protobuf-c/protobuf-c.h
)
MARK_AS_ADVANCED(PROTOBUFC_INCLUDE_DIR)

# Find the protoc Executable
FIND_PROGRAM(PROTOBUFC_PROTOC_EXECUTABLE
    NAMES protoc-c
    DOC "The Google Protocol Buffers C Compiler"
)
MARK_AS_ADVANCED(PROTOBUFC_PROTOC_EXECUTABLE)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROTOBUFC DEFAULT_MSG
    PROTOBUFC_LIBRARY PROTOBUFC_INCLUDE_DIR)

IF(PROTOBUFC_FOUND)
    SET(PROTOBUFC_INCLUDE_DIRS ${PROTOBUFC_INCLUDE_DIR})
ENDIF()
