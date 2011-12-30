#
# Copyright (C) 2009-2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
