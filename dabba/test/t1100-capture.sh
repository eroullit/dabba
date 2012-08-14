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

get_capture_thread_nr()
{
    local result_file=$1
    python -c "import yaml; y = yaml.load(open('$result_file')); print len(y['captures']);"
}

get_capture_thread_id()
{
    local thread_nr=$1
    local result_file=$2
    python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['id'];"
}

check_capture_thread_nr()
{
    local expected_thread_nr=$1
    local result_file=$2
    local result_thread_nr="$(get_capture_thread_nr $result_file)"
    test "$expected_thread_nr" = "$result_thread_nr"
}

check_capture_thread_id()
{
    local thread_nr=$1
    local result_file=$2
    local result_thread_id="$(get_capture_thread_id $thread_nr $result_file)"
    echo "$result_thread_id" | grep -w -q -E "^[0-9]+"
}

check_capture_thread_interface()
{
    local thread_nr=$1
    local expected_interface=$2
    local result_file=$3
    local result_interface="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['interface'];")"
    test "$expected_interface" = "$result_interface"
}

check_capture_thread_pcap()
{
    local thread_nr=$1
    local expected_pcap="$2"
    local result_file=$3
    local result_pcap="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['pcap'];")"
    test "$expected_pcap" = "$result_pcap"
}

check_capture_thread_packet_mmap_size()
{
    local thread_nr=$1
    local expected_packet_mmap_size=$2
    local result_file=$3
    local result_packet_mmap_size="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['packet mmap size'];")"
    test "$expected_packet_mmap_size" = "$result_packet_mmap_size"
}

check_capture_thread_frame_nr()
{
    local thread_nr=$1
    local expected_frame_nr=$2
    local result_file=$3
    local result_frame_nr="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['frame number'];")"
    test "$expected_frame_nr" = "$result_frame_nr"
}

frame_nr="16"
ring_size="$(($frame_nr * 2048))" # 2kB are allocated for one ethernet frame

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success "Check 'dabba capture' help output" "
    '$DABBA_PATH/dabba' help capture | cat <<EOF
    q
    EOF &&
    '$DABBA_PATH/dabba' capture --help | cat <<EOF
    q
    EOF
"

test_expect_success "Start capture thread on an invalid interface (too long)" "
    test_expect_code 22 '$DABBA_PATH'/dabba capture start --interface lorem-ipsum-dolor-sit --pcap test.pcap --frame-number $frame_nr
"

test_expect_success "Start capture thread on an invalid interface (does not exist)" "
    test_expect_code 19 '$DABBA_PATH'/dabba capture start --interface lorem-ipsum --pcap test.pcap --frame-number $frame_nr
"

test_expect_success "Start capture thread with a missing interface" "
    test_expect_code 22 '$DABBA_PATH'/dabba capture start --pcap test.pcap --frame-number $frame_nr
"

test_expect_success "Start capture thread with a missing pcap path" "
    test_expect_code 22 '$DABBA_PATH'/dabba capture start --interface any --frame-number $frame_nr
"

test_expect_success "Start capture thread with a missing frame number" "
    test_expect_code 22 '$DABBA_PATH'/dabba capture start --interface any --pcap test.pcap
"

test_expect_success "Invoke capture command w/o any parameters" "
    test_expect_code 38 '$DABBA_PATH'/dabba capture
"

for i in `seq 0 9`
do
        test_expect_success "Start capture thread #$(($i+1)) on loopback" "
            '$DABBA_PATH'/dabba capture start --interface any --pcap test$i.pcap --frame-number $frame_nr &&
            '$DABBA_PATH'/dabba capture list > result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) number" "
            check_capture_thread_nr $(($i+1)) result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) ID" "
            check_capture_thread_id $i result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture interface" "
            check_capture_thread_interface $i any result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture pcap file" "
            check_capture_thread_pcap $i \"$PWD/test$i.pcap\" result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture packet mmap size" "
            check_capture_thread_packet_mmap_size $i $ring_size result
        "

        test_expect_success PYTHON_YAML "Check thread #$(($i+1)) capture frame number" "
            check_capture_thread_frame_nr $i $frame_nr result
        "
done

for i in `seq 0 9`
do
        test_expect_success PYTHON_YAML "Stop capture thread #$(($i+1)) on loopback" "
            '$DABBA_PATH'/dabba capture stop --id `get_capture_thread_id $i result` &&
            '$DABBA_PATH'/dabba capture list > after &&
            test_must_fail grep -wq `get_capture_thread_id $i result` after
        "
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
