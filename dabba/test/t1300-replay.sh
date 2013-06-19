#!/bin/sh
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

test_description='Test dabba replay command'

. ./dabba-test-lib.sh

pidfile=$(mktemppid)

get_replay_thread_nr()
{
    local result_file=$1
    "$PYTHON_PATH" -c "import yaml; y = yaml.load(open('$result_file')); print len(y['replays']);"
}

check_replay_thread_nr()
{
    local expected_thread_nr=$1
    local result_file=$2
    local result_thread_nr="$(get_replay_thread_nr $result_file)"
    test "$expected_thread_nr" = "$result_thread_nr"
}

default_frame_nr="32"
frame_nr="16"
ring_size="$(($frame_nr * 2048))" # 2kB are allocated for one ethernet frame

test_expect_success "Setup: Start dabbad" "
    dabbad --daemonize --pidfile '$pidfile'
"

test_expect_success "Check 'dabba replay' help output" "
    dabba help replay | cat <<EOF
    q
    EOF &&
    dabba' replay --help | cat <<EOF
    q
    EOF
"

test_expect_failure "Start replay thread on an invalid interface (too long)" "
    test_expect_code 22 dabba replay start --interface lorem-ipsum-dolor-sit --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start replay thread on an invalid interface (does not exist)" "
    test_expect_code 19 dabba replay start --interface lorem-ipsum --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start replay thread with a missing interface" "
    test_expect_code 22 dabba replay start --pcap test.pcap --frame-number $frame_nr
"

test_expect_failure "Start replay thread with a missing pcap path" "
    test_expect_code 22 dabba replay start --interface lo --frame-number $frame_nr
"

test_expect_failure "Invoke replay command w/o any parameters" "
    test_expect_code 38 dabba replay
"

test_expect_success "Start replay thread with a default frame number" "
    dabba replay start --interface lo --pcap '$SHARNESS_TEST_DIRECTORY/t1300/http.cap' &&
    dabba replay get > result
"

test_expect_success PYTHON_YAML "Parse replay YAML output" "
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Query replay YAML output" "
    echo "$default_frame_nr" > expect_frame_number &&
    dictkeys2values replays 0 'frame number' < parsed > result_frame_number &&
    dictkeys2values replays 0 id < parsed > result_id
"

test_expect_success PYTHON_YAML "Check thread default replay frame number ($default_frame_nr)" "
    test_cmp expect_frame_number result_frame_number
"

test_expect_success PYTHON_YAML "Stop replay thread with a default frame number" "
    dabba replay stop --id '$(cat result_id)' &&
    dabba replay get > after &&
    test_must_fail grep -wq -f result_id after
"

test_expect_success "Stop all running replays thread" "
    dabba replay stop-all &&
    dabba replay get > result
"

cat > expect << EOF
---
  replays:
EOF

test_expect_success "Check that the replay list is empty" "
    test_cmp result expect
"

test_expect_success "Cleanup: Stop dabbad" "
    kill $(cat "$pidfile")
"

test_done

# vim: ft=sh:tabstop=4:et
