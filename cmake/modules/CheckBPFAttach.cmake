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

INCLUDE(CheckCSourceRuns)
      
CHECK_C_SOURCE_RUNS("
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/filter.h>

int
main(int argc, char *argv[])
{
	struct sock_fprog bpf;
	int empty;
	int sock = 0;

	memset(&bpf, 0, sizeof(bpf));

	setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
	setsockopt(sock, SOL_SOCKET, SO_DETACH_FILTER, &empty, sizeof(empty));
	exit(0);
}
"
	BPFATTACH_RUN_RESULT)

SET(HAVE_BPF_ATTACH NO)
IF(BPFATTACH_RUN_RESULT EQUAL 1)
    SET(HAVE_BPF_ATTACH YES)
    MESSAGE(STATUS "System has SO_ATTACH_FILTER/SO_DETACH_FILTER support")
ELSE(BPFATTACH_RUN_RESULT EQUAL 1)
    MESSAGE(STATUS "System does not have SO_ATTACH_FILTER/SO_DETACH_FILTER support")
ENDIF(BPFATTACH_RUN_RESULT EQUAL 1)
