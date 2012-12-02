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

test_description='Test dabba interface capabilities command'

. ./dabba-test-lib.sh

#test_expect_success 'invoke dabba interface capabilities command w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba capabilities list
#"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize --rpc
"

test_expect_success 'invoke dabba interface capabilities command with dabbad' "
    '$DABBA_PATH'/dabba interface capabilities get > result
"

test_expect_success PYTHON_YAML "Parse interface capabilities YAML output" "
    yaml2dict result > parsed
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
