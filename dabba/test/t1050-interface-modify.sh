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

test_description='Test dabba interface modify command'

. ./dabba-test-lib.sh

number_of_interface_get(){
    sed '1,2d' /proc/net/dev | wc -l | cut -f 1 -d ' '
}

#test_expect_success 'invoke dabba interface modify w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba interface modify
#"

test_expect_success DUMMY_DEV "Setup: Remove all dummy interfaces" "
    test_might_fail flush_dummy_interface
"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize --rpc
"

test_expect_success "Activate dummy interface" "
    create_dummy_interface 1
"

test_expect_success DUMMY_DEV,PYTHON_YAML "Parse interface YAML output" "
    '$DABBA_PATH'/dabba interface status get > result &&
    yaml2dict result > parsed
"

test_expect_success DUMMY_DEV "Cleanup: Remove all dummy interfaces" "
    flush_dummy_interface
"
test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
