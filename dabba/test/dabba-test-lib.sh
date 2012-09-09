#!/bin/sh
#
# Copyright (C) 2012    Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

. ./sharness.sh

DABBAD_PATH="$SHARNESS_TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$SHARNESS_TEST_DIRECTORY/../../dabba"

PYTHON_PATH="$(which python)"

modinfo dummy 2>&1 > /dev/null && test_set_prereq DUMMY_DEV
"$PYTHON_PATH" -c "import yaml" 2>&1 > /dev/null && test_set_prereq PYTHON_YAML
taskset -h 2>&1 > /dev/null && test_set_prereq TASKSET

flush_dummy_interface()
{
        sudo rmmod dummy <&6
}

create_dummy_interface()
{
        interface_nr=$1
        sudo modprobe dummy numdummies="$interface_nr" <&6
        sleep 1
}

yaml2dict()
{
    "$PYTHON_PATH" "$SHARNESS_TEST_DIRECTORY"/yaml2dict.py $@
}

dictkeys2values()
{
    "$PYTHON_PATH" "$SHARNESS_TEST_DIRECTORY"/dictkeys2values.py $@
}

# vim: ft=sh:tabstop=4:et
