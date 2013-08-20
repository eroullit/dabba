#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

# The module defines the following variables:
#  SETCAP_FOUND - true is setcap executable is found
#  SETCAP_EXECUTABLE - the path to the setcap executable
#  SETCAP_LIBRARIES - the setcap libraries

FIND_PROGRAM(SETCAP_EXECUTABLE setcap DOC "path to the setcap executable")
MARK_AS_ADVANCED(SETCAP_EXECUTABLE)

FIND_LIBRARY(SETCAP_LIBRARY NAMES cap DOC "path to libcap")
MARK_AS_ADVANCED(SETCAP_LIBRARY)
SET(SETCAP_LIBRARIES ${SETCAP_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(setcap REQUIRED_VARS SETCAP_EXECUTABLE)
