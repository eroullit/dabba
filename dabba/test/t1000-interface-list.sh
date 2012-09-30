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

number_of_interface_get(){
    sed '1,2d' /proc/net/dev | wc -l | cut -f 1 -d ' '
}

generate_list(){
        rm -f dev_list
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
    test_might_fail flush_dummy_interface
"

test_expect_success "Activate dummy interface" "
    create_dummy_interface 10
"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
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
    '$DABBA_PATH'/dabba interface list > result &&
    grep '\- name: ' result > name_result &&
    generate_yaml_list > expected &&
    sort -o expected_sorted expected &&
    sort -o result_sorted name_result &&
    test_cmp expected_sorted result_sorted
"

test_expect_success PYTHON_YAML "Parse interface list YAML output" "
    yaml2dict result > parsed
"

for i in `seq 0 $(($(number_of_interface_get)-1))`
do
    test_expect_success PYTHON_YAML "Query interface list output" "
        dictkeys2values interfaces $i name < parsed > interface_name &&
        dictkeys2values interfaces $i status < parsed > interface_status &&
        dictkeys2values interfaces $i statistics < parsed > interface_statistics
    "

    test_expect_success PYTHON_YAML "Check interface #$(($i+1)) output" "
        grep -wq -f interface_name /proc/net/dev &&
        test -n interface_status &&
        test -n interface_statistics
    "
done

test_expect_success "Activate dummy interface" "
    create_dummy_interface 1
"

test_expect_success DUMMY_DEV "Cleanup: Remove all dummy interfaces" "
    flush_dummy_interface
"
test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
