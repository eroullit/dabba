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

test_description='Test dabba interface capabilities command'

. ./dabba-test-lib.sh

ethtool_port_parse() {
    local ethtool_output="$1"
    local out="{"
    local status=""

    for port in aui mii bnc tp fibre
    do
        # append port and its related status to the output
        grep "Supported ports:" ethtool_output | grep -qi "$port"
        test $? = 0 && status="True" || status="False"
        out="${out}'$port': $status, "
    done

    # replace final comma-space by a closing curly brace
    out=$(echo "$out" | sed -e 's/, $/}/g')

    echo "$out"
}

ethtool_speed_parse() {
    local begin="$1"
    local end="$2"
    local ethtool_output="$3"
    local out="{"
    local status=""

    range_print "$begin" "$end" "$ethtool_output" | tr -d '\n' > range_output

    for speed in 10 100 1000 10000
    do
        out="$out$speed: {"
        for duplex in full half
        do
            grep -qiw "${speed}baseT/${duplex}" range_output
            test $? = 0 && status="True" || status="False"
            out="$out'$duplex': $status, "
        done
        out=$(echo "$out" | sed -e 's/, $/}, /g')
    done

    out=$(echo "$out" | sed -e 's/, $/}/g')
    echo "$out"
}

ethtool_parse() {
    local status="$(awk -F: '/'"$1"'/ {print $NF}' "$2" | tr -d ' ')"
    test "$status" = "Yes" && echo "True" || echo "False"
}

ethtool_supported_speed_parse() {
    ethtool_speed_parse "Supported link modes:" "Supported pause frame use:" "$1"
}

ethtool_supported_pause_parse() {
    ethtool_parse "Supported pause frame use" "$1"
}

ethtool_supported_autoneg_parse() {
    ethtool_parse "Supports auto-negotiation" "$1"
}

ethtool_advertised_pause_parse() {
    ethtool_parse "Advertised pause frame use" "$1"
}

ethtool_advertised_autoneg_parse() {
    ethtool_parse "Advertised auto-negotiation" "$1"
}

ethtool_lp_advertised_pause_parse() {
    ethtool_parse "Link partner advertised pause frame use" "$1"
}

ethtool_lp_advertised_autoneg_parse() {
    ethtool_parse "Link partner advertised auto-negotiation" "$1"
}

test_expect_success 'invoke dabba interface capabilities command w/o dabbad' "
    test_expect_code 22 $DABBA_PATH/dabba interface capabilities get
"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success 'invoke dabba interface capabilities command with dabbad' "
    '$DABBA_PATH'/dabba interface capabilities get > result
"

test_expect_success PYTHON_YAML "Parse interface capabilities YAML output" "
    yaml2dict result > parsed
"

for i in `seq 0 $(($(number_of_interface_get)-1))`
do
    test_expect_success PYTHON_YAML "Query interface capabilities output" "
        dictkeys2values interfaces $i name < parsed > output_name &&
        dictkeys2values interfaces $i capabilities port < parsed > output_port &&
        dictkeys2values interfaces $i capabilities supported speed < parsed > output_supported_speed &&
        dictkeys2values interfaces $i capabilities supported pause < parsed > output_supported_pause &&
        dictkeys2values interfaces $i capabilities supported autoneg < parsed > output_supported_autoneg &&
        dictkeys2values interfaces $i capabilities advertised pause < parsed > output_advertised_pause &&
        dictkeys2values interfaces $i capabilities advertised autoneg < parsed > output_advertised_autoneg &&
        dictkeys2values interfaces $i capabilities 'link-partner advertised' pause < parsed > output_lp_advertised_pause &&
        dictkeys2values interfaces $i capabilities 'link-partner advertised' autoneg < parsed > output_lp_advertised_autoneg
    "

    dev=$(cat output_name)

    test_expect_success ETHTOOL,PYTHON_YAML "Query interface '$dev' capabilities via ethtool" "
        test_might_fail '$ETHTOOL_PATH' '$dev' > ethtool_output
    "

    for feature in port supported_speed supported_pause supported_autoneg advertised_pause advertised_autoneg lp_advertised_pause lp_advertised_autoneg
    do
        test_expect_success ETHTOOL,PYTHON_YAML "Parse '$dev' $feature" "
            ethtool_${feature}_parse ethtool_output > ethtool_${feature}_parsed
        "

        test_expect_success ETHTOOL,PYTHON_YAML "Check '$dev' $feature" "
            test_cmp ethtool_${feature}_parsed output_$feature
        "
    done
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
