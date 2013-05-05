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

test_mtu=1234
test_dev="dummy0"

test_expect_success 'invoke dabba interface settings command w/o dabbad' "
    test_expect_code 22 $DABBA_PATH/dabba interface settings get
"

test_expect_success DUMMY_DEV "Setup: Remove all dummy interfaces" "
    test_might_fail flush_dummy_interface
"

test_expect_success DUMMY_DEV "Activate dummy interface" "
    create_dummy_interface 1
"

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

test_expect_success DUMMY_DEV "Fetch dummy interface settings" "
    echo '$test_dev' > test_dev &&
    sys_class_net_get test_dev mtu > sys_mtu
"

test_expect_success DUMMY_DEV "Modify interface '$test_dev' maximum transfer unit" "
    '$DABBA_PATH'/dabba interface settings modify --id '$test_dev' --mtu '$test_mtu'
"

test_expect_success DUMMY_DEV "Fetch interface '$test_dev' settings" "
    '$DABBA_PATH'/dabba interface settings get --id '$test_dev' > result
"

test_expect_success DUMMY_DEV,PYTHON_YAML "Parse interface settings YAML output" "
    yaml2dict result > parsed
"

test_expect_success DUMMY_DEV,PYTHON_YAML "Query interface '$test_dev' settings output" "
    dictkeys2values interfaces 0 name < parsed > result_name &&
    dictkeys2values interfaces 0 settings mtu < parsed > result_mtu
"

test_expect_success DUMMY_DEV,PYTHON_YAML "Check new MTU settings" "
    echo '$test_mtu' > expect_mtu
    test_cmp expect_mtu result_mtu
"

test_expect_success DUMMY_DEV "Cleanup: Restore interface '$test_dev' maximum transfer unit" "
    '$DABBA_PATH'/dabba interface settings modify --id '$test_dev' --mtu `cat sys_mtu`
"

test_expect_success DUMMY_DEV "Cleanup: Remove all dummy interfaces" "
    flush_dummy_interface
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
