#!/bin/sh
#
# Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

test_description='Test dabba list command'

. ./sharness.sh

interface_nr=100

DABBAD_PATH="$TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$TEST_DIRECTORY/../../dabba"

modinfo dummy 2>&1 > /dev/null && test_set_prereq DUMMY_DEV

flush_test_interface()
{
        sudo rmmod dummy
}

create_test_interface()
{
        sudo modprobe dummy numdummies=$interface_nr
}

generate_list(){
        rm dev_list
        for dev in `sed '1,2d' /proc/net/dev | awk -F ':' '{ print $1 }' | tr -d ' '`
        do
            echo "    - $dev" >> dev_list
        done
}

generate_yaml_list()
{
generate_list

cat <<EOF
---
  interfaces:
`cat dev_list`
EOF
}

#test_expect_success 'invoke dabba list w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba list
#"

test_expect_success DUMMY_DEV "Setup: Remove all dummy interfaces" "
    test_might_fail flush_test_interface
"

test_expect_success "invoke dabba list with dabbad" "
    $DABBAD_PATH/dabbad --daemonize &&
    sleep 0.1 &&
    $DABBA_PATH/dabba list > result &&
    killall dabbad &&
    generate_yaml_list > expected &&
    sort -o expected_sorted expected &&
    sort -o result_sorted result &&
    test_cmp expected_sorted result_sorted
"

test_expect_success DUMMY_DEV "Setup: Create $interface_nr dummy interfaces" "
    create_test_interface
"

test_expect_success DUMMY_DEV "invoke dabba list with dabbad with $interface_nr extra interfaces" "
    $DABBAD_PATH/dabbad --daemonize &&
    sleep 0.1 &&
    $DABBA_PATH/dabba list > result &&
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
