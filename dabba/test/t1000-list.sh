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

test_description='Test dabba list command'

. ./sharness.sh

DABBAD_PATH="$TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$TEST_DIRECTORY/../../dabba"

generate_list(){
for dev in `sed '1,2d' /proc/net/dev | awk -F ':' '{ print $1 }' | tr -d ' '`
do
    echo "    - $dev" >> dev_list
done
}

generate_yaml_list()
{
generate_list

cat <<EOF
---
  interfaces:
`cat dev_list`
EOF
}

test_expect_failure 'invoke dabba list w/o dabbad' "$DABBA_PATH/dabba list"
test_expect_success 'invoke dabba list with dabbad' "
    $DABBAD_PATH/dabbad --daemonize &&
    sleep 0.1 &&
    $DABBA_PATH/dabba list > result &&
    killall dabbad &&
    generate_yaml_list > expected &&
    test_cmp expected result
"

test_done

# vim: ft=sh:tabstop=4:et
