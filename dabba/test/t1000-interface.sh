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

test_description='Test dabba interface list command'

. ./dabba-test-lib.sh

interface_nr=100

generate_list(){
        rm dev_list
        for dev in `sed '1,2d' /proc/net/dev | awk -F ':' '{ print $1 }' | tr -d ' '`
        do
            echo "    - name: $dev" >> dev_list
        done
}

generate_yaml_list()
{
generate_list

cat <<EOF
`cat dev_list`
EOF
}

#test_expect_success 'invoke dabba interface list w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba interface list
#"

test_expect_success DUMMY_DEV "Setup: Remove all dummy interfaces" "
    test_might_fail flush_test_interface
"

test_expect_success "Check 'dabba interface' help output" "
    '$DABBA_PATH/dabba' help interface | cat <<EOF
    q
    EOF &&
    '$DABBA_PATH/dabba' interface --help | cat <<EOF
    q
    EOF
"

test_expect_success "invoke dabba interface list with dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize &&
    sleep 0.1 &&
    '$DABBA_PATH'/dabba interface list | grep 'name: ' > result &&
    killall dabbad &&
    generate_yaml_list > expected &&
    sort -o expected_sorted expected &&
    sort -o result_sorted result &&
    test_cmp expected_sorted result_sorted
"

test_expect_success DUMMY_DEV "Setup: Create $interface_nr dummy interfaces" "
    create_test_interface $interface_nr
"

test_expect_success DUMMY_DEV "invoke dabba interface list with dabbad with $interface_nr extra interfaces" "
    '$DABBAD_PATH'/dabbad --daemonize &&
    sleep 0.1 &&
    '$DABBA_PATH'/dabba interface list | grep 'name: ' > result &&
    killall dabbad &&
    generate_yaml_list > expected &&
    sort -o expected_sorted expected &&
    sort -o result_sorted result &&
    test_cmp expected_sorted result_sorted
"

test_expect_success DUMMY_DEV "Cleanup: Remove all dummy interfaces" "
    flush_test_interface
"

test_done

# vim: ft=sh:tabstop=4:et
