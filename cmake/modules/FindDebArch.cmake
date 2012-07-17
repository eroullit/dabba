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

IF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
  FIND_PROGRAM(DPKG_CMD dpkg)
  IF(NOT DPKG_CMD)
    EXECUTE_PROCESS(COMMAND uname -p
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    MESSAGE(STATUS "Can not find dpkg in your path, default to uname -p: ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.")
  ELSE(NOT DPKG_CMD)
    EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  ENDIF(NOT DPKG_CMD)
ENDIF(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
