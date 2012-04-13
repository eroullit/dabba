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

check_capture_thread_nr()
{
    local expected_thread_nr="$1"
    local result_file="$2"
    local result_thread_nr=$(python -c "import yaml; y = yaml.load(open('$result_file')); print len(y['captures']);")
    return $(test $result_thread_nr = $expected_thread_nr)
}

test_expect_success "Setup: Start dabbad" "
    $DABBAD_PATH/dabbad --daemonize
"

for i in `seq 10`
do
        test_expect_success "Start a capture thread #$i on loopback" "
            $DABBA_PATH/dabba capture start --device lo --pcap test$i.pcap --size $ring_size &&
            $DABBA_PATH/dabba capture list > result &&
            check_capture_thread_nr $i result
        "
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
