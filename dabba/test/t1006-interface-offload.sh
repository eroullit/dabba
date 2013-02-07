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

dev_nr=$(number_of_interface_get)

#test_expect_success 'invoke dabba interface offload command w/o dabbad' "
#    test_must_fail $DABBA_PATH/dabba offload list
#"

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success 'invoke dabba interface offload command with dabbad' "
    '$DABBA_PATH'/dabba interface get offload > result
"

test_expect_success PYTHON_YAML "Parse interface offload YAML output" "
    yaml2dict result > parsed
"

for i in `seq 0 $(($dev_nr-1))`
do
    test_expect_success PYTHON_YAML "Check interface offload output key presence on device #$i" "
        dictkeys2values interfaces $i name < parsed &&
        dictkeys2values interfaces $i offload 'rx checksum' < parsed &&
        dictkeys2values interfaces $i offload 'tx checksum' < parsed &&
        dictkeys2values interfaces $i offload 'scatter gather' < parsed &&
        dictkeys2values interfaces $i offload 'tcp segment' < parsed &&
        dictkeys2values interfaces $i offload 'udp fragment' < parsed &&
        dictkeys2values interfaces $i offload 'generic segmentation' < parsed &&
        dictkeys2values interfaces $i offload 'generic receive' < parsed &&
        dictkeys2values interfaces $i offload 'rx hashing' < parsed
    "
done

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
