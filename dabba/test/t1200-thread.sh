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

test_description='Test dabba thread command'

. ./dabba-test-lib.sh

get_default_cpu_affinity()
{
    taskset -pc 1 | awk '{ print $NF }'
}

get_default_sched_prio_min()
{
    local policy="$1"
    chrt -m | grep -i "$policy" |  awk '{ print $NF }' | cut -d/ -f1
}

get_default_sched_prio_max()
{
    local policy="$1"
    chrt -m | grep -i "$policy" |  awk '{ print $NF }' | cut -d/ -f2
}

get_thread_nr()
{
    local result_file=$1
    "$PYTHON_PATH" -c "import yaml; y = yaml.load(open('$result_file')); print len(y['threads']);"
}

check_thread_nr()
{
    local expected_thread_nr=$1
    local result_file=$2
    local result_thread_nr="$(get_thread_nr $result_file)"
    test "$expected_thread_nr" = "$result_thread_nr"
}

default_cpu_affinity=$(get_default_cpu_affinity)

test_expect_success "Setup: Start dabbad" "
    '$DABBAD_PATH'/dabbad --daemonize
"

test_expect_success "Setup: Start a basic capture on loopback" "
    '$DABBA_PATH'/dabba capture start --interface any --pcap test.pcap --frame-number 8
"

test_expect_success "Check 'dabba thread' help output" "
    '$DABBA_PATH/dabba' help thread | cat <<EOF
    q
    EOF &&
    '$DABBA_PATH/dabba' thread --help | cat <<EOF
    q
    EOF
"

test_expect_success "Fetch running thread information" "
    '$DABBA_PATH'/dabba thread list > result &&
    yaml2dict result > parsed
"

test_expect_success PYTHON_YAML "Check thread number" "
    check_thread_nr 1 result
"

test_expect_success PYTHON_YAML "Check thread ID" "
    echo \"$(dictkeys2values threads 0 id < parsed)\" | grep -wq -E '^[0-9]+'
"

test_expect_success PYTHON_YAML "Check thread type" "
    test \"$(dictkeys2values threads 0 type < parsed)\" = capture
"

test_expect_success PYTHON_YAML "Check thread default scheduling policy" "
    test \"$(dictkeys2values threads 0 'scheduling policy' < parsed)\" = other
"

test_expect_success PYTHON_YAML "Check thread default scheduling priority" "
    test \"$(dictkeys2values threads 0 'scheduling priority' < parsed)\" = 0
"

test_expect_success TASKSET,PYTHON_YAML "Check thread default CPU affinity" "
    test \"$(dictkeys2values threads 0 'cpu affinity' < parsed)\" = '$default_cpu_affinity'
"

thread_id="$(dictkeys2values threads 0 id < parsed)"

for policy in fifo rr other
do
        min_prio=$(get_default_sched_prio_min $policy)
        max_prio=$(get_default_sched_prio_max $policy)

        for priority in $min_prio $max_prio
        do
                test_expect_success PYTHON_YAML "Modify capture thread scheduling policy ($policy:$priority)" "
                    '$DABBA_PATH'/dabba thread modify --sched-policy '$policy' --sched-prio '$priority' --id '$thread_id'
                "

                test_expect_success PYTHON_YAML "Parse thread YAML output" "
                    '$DABBA_PATH'/dabba thread list > result &&
                    yaml2dict result > parsed
                "

                test_expect_success PYTHON_YAML "Check new capture thread scheduling policy" "
                    test \"$(dictkeys2values threads 0 'scheduling policy' < parsed)\" = '$policy' &&
                    test \"$(dictkeys2values threads 0 'scheduling priority' < parsed)\" = '$priority'
                "
        done

        for priority in $(($min_prio-1)) $(($max_prio+1))
        do
                test_expect_success PYTHON_YAML "Do not modify capture thread out-of-range scheduling policy ($policy:$priority)" "
                    test_must_fail '$DABBA_PATH'/dabba thread modify --sched-policy '$policy' --sched-prio '$priority' --id '$thread_id'
                "

                test_expect_success PYTHON_YAML "Parse thread YAML output" "
                    '$DABBA_PATH'/dabba thread list > result &&
                    yaml2dict result > parsed
                "

                test_expect_success PYTHON_YAML "Check that the capture thread scheduling policy did not change" "
                    test \"$(dictkeys2values threads 0 'scheduling policy' < parsed)\" = '$policy' &&
                    test \"$(dictkeys2values threads 0 'scheduling priority' < parsed)\" != '$priority'
                "
        done
done

for cpu_affinity in 0 $default_cpu_affinity
do
        test_expect_success PYTHON_YAML "Modify capture thread CPU affinity (run on CPU $cpu_affinity)" "
            '$DABBA_PATH'/dabba thread modify --cpu-affinity '$cpu_affinity' --id '$thread_id'
        "

        test_expect_success PYTHON_YAML "Parse thread YAML output" "
            '$DABBA_PATH'/dabba thread list > result &&
            yaml2dict result > parsed
        "

        test_expect_success TASKSET,PYTHON_YAML "Check thread modified CPU affinity" "
            test \"$(dictkeys2values threads 0 'cpu affinity' < parsed)\" = '$cpu_affinity'
        "
done

test_expect_success PYTHON_YAML "Stop capture thread using thread output" "
    '$DABBA_PATH'/dabba capture stop --id '$thread_id' &&
    '$DABBA_PATH'/dabba thread list > after &&
    test_must_fail grep -wq '$thread_id' after
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
