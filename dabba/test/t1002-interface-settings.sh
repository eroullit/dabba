#!/bin/sh
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

test_description='Test dabba interface settings command'

. ./dabba-test-lib.sh

pidfile=$(mktemppid)
test_value=1234

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface settings command w/o dabbad' "
    test_expect_code 22 dabba interface settings get
"

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize --pidfile '$pidfile'
"

test_expect_success 'invoke dabba interface settings command with dabbad' "
    dabba interface settings get > result
"

test_expect_success PYTHON_YAML "Parse interface settings YAML output" "
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Check interface settings output length" "
    test $(number_of_interface_get) -eq $(yaml_number_of_interface_get parsed)
"

for i in `seq 0 $(($(yaml_number_of_interface_get parsed)-1))`
do
    test_expect_success PYTHON_YAML "Query interface settings output" "
        dictkeys2values interfaces $i name < parsed > output_name &&
        dictkeys2values interfaces $i settings speed < parsed > output_speed &&
        dictkeys2values interfaces $i settings mtu < parsed > output_mtu &&
        dictkeys2values interfaces $i settings duplex < parsed > output_duplex &&
        dictkeys2values interfaces $i settings 'txqlen' < parsed > output_qlen
    "

    test_expect_success PYTHON_YAML "Query interface settings via /sys/class/net" "
        sys_class_net_get output_name speed > sys_speed &&
        sys_class_net_get output_name mtu > sys_mtu &&
        sys_class_net_get output_name duplex > sys_duplex &&
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

for item in mtu txqlen
do
    test_expect_success TEST_DEV "Fetch test interface settings" "
        echo '$TEST_DEV' > test_dev &&
        sys_class_net_get test_dev '$item' > 'sys_$item'
    "

    test_expect_success TEST_DEV "Modify test interface $item" "
        dabba interface settings modify --id '$TEST_DEV' --$item '$test_value'
    "

    test_expect_success TEST_DEV "Fetch test interface settings" "
        dabba interface settings get --id '$TEST_DEV' > result
    "

    test_expect_success TEST_DEV,PYTHON_YAML "Parse test interface settings YAML output" "
        yaml2dict result > parsed
    "

    test_expect_success TEST_DEV,PYTHON_YAML "Query test interface settings output" "
        dictkeys2values interfaces 0 name < parsed > result_name &&
        dictkeys2values interfaces 0 settings '$item' < parsed > 'result_$item'
    "

    test_expect_success TEST_DEV,PYTHON_YAML "Check test interface new $item settings" "
        echo '$test_value' > 'expect_$item' &&
        test_cmp 'expect_$item' 'result_$item'
    "
done

test_expect_success "Cleanup: Stop dabbad" "
    kill $(cat "$pidfile")
"

test_done

# vim: ft=sh:tabstop=4:et
