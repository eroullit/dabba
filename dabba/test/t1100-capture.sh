#!/bin/sh
#
# Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
ring_size=$((16*1024*1024))

. ./dabba-test-lib.sh

get_capture_thread_nr()
{
    local thread_nr="$1"
    local result_file="$2"
    python -c "import yaml; y = yaml.load(open('$result_file')); print len(y['captures']);"
}

get_capture_thread_id()
{
    local thread_nr="$1"
    local result_file="$2"
    python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['id'];"
}

check_capture_thread_nr()
{
    local expected_thread_nr="$1"
    local result_file="$2"
    local result_thread_nr="$(get_capture_thread_nr \"$result_file\")"
    return $(test "$expected_thread_nr" = "$result_thread_nr")
}

check_capture_thread_id()
{
    local thread_nr="$1"
    local result_file="$2"
    local result_thread_id="$(get_capture_thread_id $thread_nr \"$result_file\")"
    return $(echo "$result_thread_id" | grep -w -q -E "^[0-9]+")
}

check_capture_thread_interface()
{
    local thread_nr="$1"
    local expected_interface="$2"
    local result_file="$3"
    local result_interface="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['interface'];")"
    return $(test "$expected_interface" = "$result_interface")
}

check_capture_thread_pcap()
{
    local thread_nr="$1"
    local expected_pcap="$2"
    local result_file="$3"
    local result_pcap="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['pcap'];")"
    return $(test "$expected_pcap" = "$result_pcap")
}

check_capture_thread_packet_mmap_size()
{
    local thread_nr="$1"
    local expected_packet_mmap_size="$2"
    local result_file="$3"
    local result_packet_mmap_size="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['captures'][$thread_nr]['packet mmap size'];")"
    return $(test "$expected_packet_mmap_size" = "$result_packet_mmap_size")
}

test_expect_success "Setup: Start dabbad" "
    $DABBAD_PATH/dabbad --daemonize
"

for i in `seq 0 9`
do
        test_expect_success "Start a capture thread #$(($i+1)) on loopback" "
            $DABBA_PATH/dabba capture start --device lo --pcap test$i.pcap --size $ring_size &&
            $DABBA_PATH/dabba capture list > result &&
            check_capture_thread_nr $i result &&
            check_capture_thread_id $i result &&
            check_capture_thread_interface $i lo result &&
            check_capture_thread_pcap $i \"$PWD/test$i.pcap\" result &&
            check_capture_thread_packet_mmap_size $i $ring_size result
        "
done

for i in `seq 10`
do
        test_expect_success "Start a capture thread #$i on loopback" "
            $DABBA_PATH/dabba capture list > result &&
            $DABBA_PATH/dabba capture stop --id `get_capture_thread_id 0 result`
        "
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
