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

test_description='Test dabba interface coalesce command'

. ./dabba-test-lib.sh

ethtool_coalesce_parse() {
    local pattern="$1"
    local ethtool_output="$2"

    test $(grep -c "$pattern" "$ethtool_output") -gt 0 && \
    awk '/'$pattern'/ {print $NF}' "$ethtool_output" || echo "0"
}

ethtool_coalesce_name_get() {
    case "$1" in
        "packet rate high") echo "pkt-rate-high";;
        "packet rate low") echo "pkt-rate-low";;
        "rx max frame normal") echo "rx-frames";;
        "rx max frame irq") echo "rx-frames-irq";;
        "rx max frame high") echo "rx-frames-high";;
        "rx max frame low") echo "rx-frames-low";;
        "rx usec normal") echo "rx-usec";;
        "rx usec irq") echo "rx-usec-irq";;
        "rx usec high") echo "rx-usec-high";;
        "rx usec low") echo "rx-usec-low";;
        "tx max frame normal") echo "tx-frames";;
        "tx usec normal") echo "tx-usec";;
        "tx max frame irq") echo "tx-frames-irq";;
        "tx max frame high") echo "tx-frames-high";;
        "tx max frame low") echo "tx-frames-low";;
        "tx usec irq") echo "tx-usec-irq";;
        "tx usec high") echo "tx-usec-high";;
        "tx usec low") echo "tx-usec-low";;
    esac
}

test_expect_success "Setup: Stop already running dabbad" "
    test_might_fail killall dabbad
"

test_expect_success 'invoke dabba interface coalesce command w/o dabbad' "
    test_expect_code 22 $DABBA_PATH/dabba interface coalesce get
"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success 'invoke dabba interface coalesce command with dabbad' "
    '$DABBA_PATH'/dabba interface coalesce get > result
"

test_expect_success PYTHON_YAML "Parse interface coalesce YAML output" "
    yaml2dict result > parsed
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_expect_success PYTHON_YAML "Check interface coalesce output length" "
    test $(number_of_interface_get) -eq $(yaml_number_of_interface_get parsed)
"

for i in `seq 0 $(($(yaml_number_of_interface_get parsed)-1))`
do
    test_expect_success PYTHON_YAML "Query interface capabilities output" "
        dictkeys2values interfaces $i name < parsed > output_name
    "

    dev=$(cat output_name 2>/dev/null)

    test_expect_success ETHTOOL,PYTHON_YAML "Query interface '$dev' capabilities via ethtool" "
        test_might_fail '$ETHTOOL_PATH' --show-coalesce '$dev' > ethtool_output
    "

    for type in high low
    do
        ethtool_pattern="$(ethtool_coalesce_name_get "packet rate $type")"

        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' packet rate $type" "
            test $(ethtool_coalesce_parse "$ethtool_pattern" ethtool_output) = \
            $(dictkeys2values interfaces $i coalesce "packet rate $type" < parsed)
        "
    done

    for direction in rx tx
    do
        for feature in usec 'max frame'
        do
            for type in irq high low normal
            do
                ethtool_pattern="$(ethtool_coalesce_name_get "$direction $feature $type")"
                test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $direction $feature $type" "
                    test $(ethtool_coalesce_parse "$ethtool_pattern" ethtool_output) = \
                    $(dictkeys2values interfaces $i coalesce "$direction" "$feature" "$type" < parsed)
                "
            done
        done
    done
done

test_done

# vim: ft=sh:tabstop=4:et
