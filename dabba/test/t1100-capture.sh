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

test_description='Test dabba capture command'

. ./dabba-test-lib.sh

pidfile=$(mktemppid)

get_capture_thread_nr()
{
    local result_file=$1
    "$PYTHON_PATH" -c "import yaml; y = yaml.load(open('$result_file')); print len(y['captures']);"
}

check_capture_thread_nr()
{
    local expected_thread_nr=$1
    local result_file=$2
    local result_thread_nr="$(get_capture_thread_nr $result_file)"
    test "$expected_thread_nr" = "$result_thread_nr"
}

default_frame_nr="32"
frame_nr="16"
ring_size="$(($frame_nr * 2048))" # 2kB are allocated for one ethernet frame

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize --pidfile '$pidfile'
"

test_expect_success "Check 'dabba capture' help output" "
    dabba help capture | cat <<EOF
    q
    EOF &&
    dabba' capture --help | cat <<EOF
    q
    EOF
"

test_expect_failure "Start capture thread on an invalid interface (too long)" "
    test_expect_code 22 dabba capture start --interface lorem-ipsum-dolor-sit --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start capture thread on an invalid interface (does not exist)" "
    test_expect_code 19 dabba capture start --interface lorem-ipsum --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start capture thread with a missing interface" "
    test_expect_code 22 dabba capture start --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start capture thread with a missing pcap path" "
    test_expect_code 22 dabba capture start --interface any --frame-number $frame_nr
"

test_expect_failure "Invoke capture command w/o any parameters" "
    test_expect_code 38 dabba capture
"

test_expect_success "Start capture thread with a default frame number" "
    dabba capture start --interface any --pcap test.pcap &&
    dabba capture get > result
"

test_expect_success PYTHON_YAML "Parse capture YAML output" "
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Query capture YAML output" "
    echo "$default_frame_nr" > expect_frame_number &&
    dictkeys2values captures 0 'frame number' < parsed > result_frame_number &&
    dictkeys2values captures 0 id < parsed > result_id
"

test_expect_success PYTHON_YAML "Check thread default capture frame number ($default_frame_nr)" "
    test_cmp expect_frame_number result_frame_number
"

test_expect_success PYTHON_YAML "Stop capture thread with a default frame number" "
    dabba capture stop --id '$(cat result_id)' &&
    dabba capture get > after &&
    test_must_fail grep -wq -f result_id after
"

for i in `seq 0 9`
do
        test_expect_success "Start capture thread #$(($i+1)) on all interfaces" "
            dabba capture start --interface any --pcap test$i.pcap --frame-number $frame_nr &&
            dabba capture get > result
        "

        test_expect_success PYTHON_YAML "Parse capture YAML output" "
            yaml2dict result > parsed
        "

        test_expect_success PYTHON_YAML "Query capture YAML output" "
            echo 'any' > expect_interface &&
            echo '$SHARNESS_TRASH_DIRECTORY/test$i.pcap' > expect_pcap &&
            echo '$ring_size' > expect_packet_mmap_size &&
            echo '$frame_nr' > expect_frame_number &&
            dictkeys2values captures $i id < parsed > result_id &&
            dictkeys2values captures $i interface < parsed > result_interface &&
            dictkeys2values captures $i pcap < parsed > result_pcap &&
            dictkeys2values captures $i 'frame number' < parsed > result_frame_number &&
            dictkeys2values captures $i 'packet mmap size' < parsed > result_packt_mmap_size
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) number" "
            check_capture_thread_nr $(($i+1)) result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) ID" "
            grep -q -E '^[0-9]+' result_id
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture interface" "
            test_cmp expect_interface result_interface
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture pcap file" "
            test_cmp expect_pcap result_pcap
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture packet mmap size" "
            test_cmp expect_packet_mmap_size result_packt_mmap_size
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture frame number" "
            test_cmp expect_frame_number result_frame_number
        "
done

test_expect_success "Generate some traffic to capture" "
    ping -c 10 -i 0.2 -s 1500 localhost
"

test_expect_success PYTHON_YAML "Query capture thread id to stop" "
    dictkeys2values captures 0 id < parsed > result_id
"

test_expect_success PYTHON_YAML "Stop capture thread #0 on loopback" "
    dabba capture stop --id '$(cat result_id)' &&
    dabba capture get > after &&
    test_must_fail grep -wq -f result_id after
"

test_expect_success "Measure pcap file size before appending" "
    stat -c %s test0.pcap > before_size
"

test_expect_success "Start a capture with pcap append" "
    dabba capture start --interface any --pcap test0.pcap --append
"

test_expect_success "Generate some traffic to capture" "
    ping -c 10 -i 0.2 -s 1500 localhost
"

test_expect_success "Measure pcap file size after appending" "
    stat -c %s test0.pcap > after_size
"

test_expect_success "Check that appended pcap file size grows" "
    test $(cat after_size) -gt $(cat before_size)
"

test_expect_success "Stop all running captures thread" "
    dabba capture stop-all &&
    dabba capture get > result
"

cat > expect << EOF
---
  captures:
EOF

test_expect_success "Check that the capture list is empty" "
    test_cmp result expect
"

test_expect_success "Cleanup: Stop dabbad" "
    kill $(cat "$pidfile")
"

test_done

# vim: ft=sh:tabstop=4:et
