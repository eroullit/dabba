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

test_description='Test dabba interface pause command'

. ./dabba-test-lib.sh

dev_nr=$(number_of_interface_get)

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface pause command w/o dabbad' "
    test_expect_code 22 $DABBA_PATH/dabba interface pause get 
"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success 'invoke dabba interface pause command with dabbad' "
    '$DABBA_PATH'/dabba interface pause get > result
"

test_expect_success PYTHON_YAML "Parse interface pause YAML output" "
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Check interface pause output length" "
    test '$dev_nr' -eq $(yaml_number_of_interface_get parsed)
"

for i in `seq 0 $(($dev_nr-1))`
do
    test_expect_success PYTHON_YAML "Check interface pause output key presence on device #$i" "
        dictkeys2values interfaces $i name < parsed > output_name
    "

    dev=$(cat output_name 2>/dev/null)

    test_expect_success ETHTOOL,PYTHON_YAML "Query interface '$dev' pause via ethtool" "
        test_might_fail '$ETHTOOL_PATH' --show-pause '$dev' > ethtool_output
    "

    for feature in rx tx autoneg
    do
        test_expect_success PYTHON_YAML "Parse '$dev' $feature pause settings from YAML output" "
            dictkeys2values interfaces $i pause '$feature' < parsed > 'output_$feature'
        "

        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $feature pause settings" "
            ethtool_status_parse '$feature' ethtool_output > 'ethtool_${feature}_parsed'
        "

        test_expect_success ETHTOOL,PYTHON_YAML "Check '$dev' $feature pause settings" "
            test_cmp 'ethtool_${feature}_parsed' 'output_$feature'
        "
    done
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
