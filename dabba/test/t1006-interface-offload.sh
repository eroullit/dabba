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

test_description='Test dabba interface offload command'

. ./dabba-test-lib.sh

ethtool_offload_long_name_get() {
    case "$1" in
        "rx-csum") echo "rx-checksumming";;
        "tx-csum") echo "tx-checksumming";;
        "sg") echo "scatter-gather";;
        "tso") echo "tcp-segmentation-offload";;
        "ufo") echo "udp-fragmentation-offload";;
        "gso") echo "generic-segmentation-offload";;
        "gro") echo "generic-receive-offload";;
        "lro") echo "large-receive-offload";;
        "rx-hash") echo "receive-hashing";;
    esac
}

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface offload command w/o dabbad' "
    test_expect_code 22 dabba interface offload get
"

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize
"

test_expect_success 'invoke dabba interface offload command with dabbad' "
    dabba interface offload get > result
"

test_expect_success PYTHON_YAML "Parse interface offload YAML output" "
    yaml2dict result > parsed
"

for i in `seq 0 $(($(yaml_number_of_interface_get parsed)-1))`
do
    test_expect_success PYTHON_YAML "Check interface offload output key presence on device #$i" "
        dictkeys2values interfaces $i name < parsed > output_name
    "

    dev="$(cat output_name 2>/dev/null)"

    test_expect_success ETHTOOL,PYTHON_YAML "Query interface '$dev' pause via ethtool" "
        test_might_fail '$ETHTOOL_PATH' --show-offload '$dev' > ethtool_output
    "

    for feature in rx-csum tx-csum sg tso ufo gso gro rx-hash
    do
        test_expect_success PYTHON_YAML "Check interface '$dev' $feature offload" "
            dictkeys2values interfaces $i offload '$feature' < parsed > 'output_$feature'
        "

        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $feature offload settings" "
            ethtool_status_parse '$(ethtool_offload_long_name_get $feature)' ethtool_output > 'ethtool_${feature}_parsed'
        "

        test_expect_success ETHTOOL,PYTHON_YAML "Check '$dev' $feature offload settings" "
            test_cmp 'ethtool_${feature}_parsed' 'output_$feature'
        "
    done
done

test_expect_success TEST_DEV "invoke dabba interface '$TEST_DEV' offload command with dabbad" "
    dabba interface offload get --id '$TEST_DEV' > result
"

test_expect_success TEST_DEV,PYTHON_YAML "Parse interface offload YAML output" "
    yaml2dict result > parsed
"

for feature in rx-csum tx-csum sg tso ufo gso gro rx-hash
do
    test_expect_success TEST_DEV,PYTHON_YAML "Check interface '$TEST_DEV' $feature feature" "
        dictkeys2values interfaces 0 offload '$feature' < parsed > 'output_$feature'
    "

    if [ "$(cat "output_$feature" 2> /dev/null)" = "True" ]; then
        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $feature" "
            dabba interface offload modify --id '$TEST_DEV' --'$feature' false &&
            dabba interface offload get --id '$TEST_DEV' > mod_result
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Parse modified '$TEST_DEV' offload YAML output" "
            yaml2dict mod_result > mod_parsed
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Query '$TEST_DEV' $feature status" "
            test $(dictkeys2values interfaces 0 offload "$feature" < mod_parsed) = False
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $feature to previous status" "
            dabba interface offload modify --id '$TEST_DEV' --'$feature' true
        "
    fi
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
