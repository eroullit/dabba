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

DABBAD_PATH="$TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$TEST_DIRECTORY/../../dabba"

modinfo dummy 2>&1 > /dev/null && test_set_prereq DUMMY_DEV
python -c "import yaml" 2>&1 > /dev/null && test_set_prereq PYTHON_YAML
taskset -h 2>&1 > /dev/null && test_set_prereq TASKSET

flush_test_interface()
{
        sudo rmmod dummy
}

create_test_interface()
{
        #local interface_nr=${$1:-1}
        sudo modprobe dummy numdummies="$interface_nr"
        sleep 1
}

# vim: ft=sh:tabstop=4:et
