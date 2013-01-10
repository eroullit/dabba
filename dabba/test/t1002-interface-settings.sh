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

test_description='Test dabba interface settings command'

. ./dabba-test-lib.sh

sys_class_net_get()
{
    local dev="$(cat "$1")"
    local param="$2"
    local rc

    cat "/sys/class/net/$dev/$param" > tmp_set 2> /dev/null
    rc=$?

    (test -s tmp_set && test $rc = 0 && cat tmp_set) || echo "0"
}

#test_expect_success 'invoke dabba interface settings command w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba settings list
#"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success 'invoke dabba interface settings command with dabbad' "
    '$DABBA_PATH'/dabba interface settings get > result
"

test_expect_success PYTHON_YAML "Parse interface settings YAML output" "
    yaml2dict result > parsed
"

for i in `seq 0 $(($(number_of_interface_get)-1))`
do
    test_expect_success PYTHON_YAML "Query interface settings output" "
        dictkeys2values interfaces $i name < parsed > output_name &&
        dictkeys2values interfaces $i settings speed < parsed > output_speed
        dictkeys2values interfaces $i settings mtu < parsed > output_mtu
        dictkeys2values interfaces $i settings duplex < parsed > output_duplex
        dictkeys2values interfaces $i settings 'tx qlen' < parsed > output_qlen
    "

    test_expect_success PYTHON_YAML "Query interface settings via /sys/class/net" "
        sys_class_net_get output_name speed > sys_speed
        sys_class_net_get output_name mtu > sys_mtu
        sys_class_net_get output_name duplex > sys_duplex
        sys_class_net_get output_name tx_queue_len > sys_qlen
    "

    test_expect_success PYTHON_YAML "Compare speed from /sys interface" "
        test_cmp sys_speed output_speed
    "

    test_expect_success PYTHON_YAML "Compare mtu from /sys interface" "
        test_cmp sys_mtu output_mtu
    "

    # how to cope with interface which do not have duplex settings
    test_expect_success PYTHON_YAML "Compare duplex from /sys interface" "
        test_might_fail test_cmp sys_duplex output_duplex
    "

    test_expect_success PYTHON_YAML "Compare tx queue length from /sys interface" "
        test_cmp sys_qlen output_qlen
    "

done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
