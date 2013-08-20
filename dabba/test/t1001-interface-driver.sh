#!/bin/sh
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

test_description='Test dabba interface driver command'

. ./dabba-test-lib.sh

pidfile=$(mktemppid)

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface driver command w/o dabbad' "
    test_expect_code 22 dabba interface driver get
"

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize --pidfile '$pidfile'
"

test_expect_success 'invoke dabba interface driver command with dabbad' "
    dabba interface driver get > result
"

test_expect_success PYTHON_YAML "Parse interface driver YAML output" "
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Check interface driver output length" "
    test $(number_of_interface_get) -eq $(yaml_number_of_interface_get parsed)
"

for i in `seq 0 $(($(yaml_number_of_interface_get parsed)-1))`
do
    test_expect_success PYTHON_YAML "Check interface driver output key presence on device #$i" "
        dictkeys2values interfaces $i name < parsed &&
        dictkeys2values interfaces $i 'driver name' < parsed &&
        dictkeys2values interfaces $i 'driver version' < parsed &&
        dictkeys2values interfaces $i 'bus info' < parsed &&
        dictkeys2values interfaces $i 'firmware version' < parsed
    "
done

test_expect_success "Cleanup: Stop dabbad" "
    kill $(cat "$pidfile")
"

test_done

# vim: ft=sh:tabstop=4:et
