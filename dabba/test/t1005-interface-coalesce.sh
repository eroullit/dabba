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

    test $(grep -cw "$pattern:" "$ethtool_output") -gt 0 && \
    awk '/^'"$pattern"':/ {print $NF}' "$ethtool_output" || echo "0"
}

ethtool_adaptive_coalesce_parse() {
    local pattern="$1"
    local ethtool_output="$2"

    grep Adaptive "$ethtool_output"| sed 's/Adaptive //g' | sed 's/  /\n/g' > adaptive_output

    status=$(awk 'tolower($1) ~ /'"$pattern"'/ {print $NF}' adaptive_output)

    test "$status" = "on" && echo "True" || echo "False"
}

ethtool_coalesce_name_get() {
    case "$1" in
        "packet rate high") echo "pkt-rate-high";;
        "packet rate low") echo "pkt-rate-low";;
        "rate sample interval") echo "sample-interval";;
        "stats block") echo "stats-block-usecs";;
        "rx adaptive") echo "adaptive-rx";;
        "rx max frame normal") echo "rx-frames";;
        "rx max frame irq") echo "rx-frames-irq";;
        "rx max frame high") echo "rx-frames-high";;
        "rx max frame low") echo "rx-frames-low";;
        "rx usec normal") echo "rx-usecs";;
        "rx usec irq") echo "rx-usecs-irq";;
        "rx usec high") echo "rx-usecs-high";;
        "rx usec low") echo "rx-usecs-low";;
        "tx adaptive") echo "adaptive-tx";;
        "tx max frame normal") echo "tx-frames";;
        "tx usec normal") echo "tx-usecs";;
        "tx max frame irq") echo "tx-frames-irq";;
        "tx max frame high") echo "tx-frames-high";;
        "tx max frame low") echo "tx-frames-low";;
        "tx usec irq") echo "tx-usecs-irq";;
        "tx usec high") echo "tx-usecs-high";;
        "tx usec low") echo "tx-usecs-low";;
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

    for feature in 'packet rate high' 'packet rate low' 'rate sample interval' 'stats block'
    do
        ethtool_pattern="$(ethtool_coalesce_name_get "$feature")"

        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $feature" "
            test $(ethtool_coalesce_parse "$ethtool_pattern" ethtool_output) = \
            $(dictkeys2values interfaces $i coalesce "$feature" < parsed)
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

        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $direction adaptive" "
            test $(ethtool_adaptive_coalesce_parse "$direction" ethtool_output) = \
            $(dictkeys2values interfaces $i coalesce "$direction" adaptive < parsed)
        "
    done
done

test_expect_success TEST_DEV "Fetch '$TEST_DEV' coalesce settings" "
    '$DABBA_PATH'/dabba interface coalesce get --id '$TEST_DEV' > result
"

test_expect_success TEST_DEV,PYTHON_YAML "Parse '$TEST_DEV' coalesce YAML output" "
    yaml2dict result > parsed
"

for feature in 'packet rate high' 'packet rate low' 'rate sample interval' 'stats block'
do
    ethtool_pattern="$(ethtool_coalesce_name_get "$feature")"
    value=$(dictkeys2values interfaces 0 coalesce "$feature" < parsed)

    if [ "$value" != "0" ]; then
        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $feature value" "
            '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' 50 &&
            '$DABBA_PATH'/dabba interface coalesce get --id '$TEST_DEV' > mod_result
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Parse modified '$TEST_DEV' coalesce YAML output" "
            yaml2dict mod_result > mod_parsed
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Query '$TEST_DEV' $feature output" "
            test $(dictkeys2values interfaces 0 coalesce "$direction" "$feature" "$type" < mod_parsed) = '50'
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $feature to previous value" "
            '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' '$value'
        "
    fi
done

for direction in rx tx
do
    for feature in usec 'max frame'
    do
        for type in irq high low normal
        do
            ethtool_pattern="$(ethtool_coalesce_name_get "$direction $feature $type")"
            value=$(dictkeys2values interfaces 0 coalesce "$direction" "$feature" "$type" < parsed)

            if [ "$value" != "0" ]; then
                test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $direction $feature $type value" "
                    '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' 50 &&
                    '$DABBA_PATH'/dabba interface coalesce get --id '$TEST_DEV' > mod_result
                "

                test_expect_success TEST_DEV,PYTHON_YAML "Parse modified '$TEST_DEV' coalesce YAML output" "
                    yaml2dict mod_result > mod_parsed
                "

                test_expect_success TEST_DEV,PYTHON_YAML "Query '$TEST_DEV' $direction $feature $type output" "
                    test $(dictkeys2values interfaces 0 coalesce "$direction" "$feature" "$type" < mod_parsed) = '50'
                "

                test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $direction $feature $type to previous value" "
                    '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' '$value'
                "
            fi
        done
    done

    ethtool_pattern="$(ethtool_coalesce_name_get "$direction adaptive")"
    value=$(dictkeys2values interfaces 0 coalesce "$direction" adaptive < parsed)

    if [ "$value" = "True" ]; then
        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $direction adaptive value" "
            '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' false &&
            '$DABBA_PATH'/dabba interface coalesce get --id '$TEST_DEV' > mod_result
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Parse modified '$TEST_DEV' coalesce YAML output" "
            yaml2dict mod_result > mod_parsed
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Query '$TEST_DEV' $direction adaptive output" "
            test $(dictkeys2values interfaces 0 coalesce "$direction" "$feature" "$type" < mod_parsed) = '50'
        "

        test_expect_success TEST_DEV,PYTHON_YAML "Modify '$TEST_DEV' $direction adaptive to previous value" "
            '$DABBA_PATH'/dabba interface coalesce modify --id '$TEST_DEV' --'$ethtool_pattern' '$value'
        "
    fi
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
