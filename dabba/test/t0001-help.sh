#!/bin/sh
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

test_description='Test dabba help output'

. ./dabba-test-lib.sh

cat > expect <<EOF
usage: dabba [--help] [--version] <command> [<subcommand>] <action> [<args>]


The most commonly used dabba commands are:
   interface   perform an interface related command
   thread      perform a thread related command
   capture     capture live traffic from an interface

See 'dabba help <command> [<subcommand>]' for more specific information.
EOF

test_expect_success "Check dabba help output" "
    dabba --help > result &&
    test_cmp expect result
"

test_done

# vim: ft=sh:tabstop=4:et
