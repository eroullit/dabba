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

test_description='Test dabba interface driver command'

. ./dabba-test-lib.sh

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface driver command w/o dabbad' "
    test_expect_code 22 dabba interface driver get
"

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize
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
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
