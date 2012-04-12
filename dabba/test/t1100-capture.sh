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

capture_thread_nr_get()
{
    local file="$1"
    python -c "import yaml; y = yaml.load(open('$file')); print len(y['captures']);"
}

test_expect_success "Setup: Start dabbad" "
    $DABBAD_PATH/dabbad --daemonize
"

test_expect_success "Start a capture thread on loopback" "
    $DABBA_PATH/dabba capture start --device lo --pcap test1.pcap --size $ring_size &&
    $DABBA_PATH/dabba capture list > result &&
    test `capture_thread_nr_get result` = 1
"

test_expect_success "Start a second capture thread on loopback" "
    $DABBA_PATH/dabba capture start --device lo --pcap test2.pcap --size $ring_size &&
    $DABBA_PATH/dabba capture list > result &&
    test `capture_thread_nr_get result` = 2
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
