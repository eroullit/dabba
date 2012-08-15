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

get_thread_nr()
{
    local result_file=$1
    python -c "import yaml; y = yaml.load(open('$result_file')); print len(y['threads']);"
}

get_thread_id()
{
    local thread_nr=$1
    local result_file=$2
    python -c "import yaml; y = yaml.load(open('$result_file')); print y['threads'][$thread_nr]['id'];"
}

check_thread_nr()
{
    local expected_thread_nr=$1
    local result_file=$2
    local result_thread_nr="$(get_thread_nr $result_file)"
    test "$expected_thread_nr" = "$result_thread_nr"
}

check_thread_id()
{
    local thread_nr=$1
    local result_file=$2
    local result_thread_id="$(get_thread_id $thread_nr $result_file)"
    echo "$result_thread_id" | grep -w -q -E "^[0-9]+"
}

check_thread_type()
{
    local thread_nr=$1
    local expected_type=$2
    local result_file=$3
    local result_type="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['threads'][$thread_nr]['type'];")"
    test "$expected_type" = "$result_type"
}

check_thread_sched_prio()
{
    local thread_nr=$1
    local expected_sched_prio="$2"
    local result_file=$3
    local result_sched_prio="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['threads'][$thread_nr]['scheduling priority'];")"
    test "$expected_sched_prio" = "$result_sched_prio"
}

check_thread_sched_policy()
{
    local thread_nr=$1
    local expected_sched_policy=$2
    local result_file=$3
    local result_sched_policy="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['threads'][$thread_nr]['scheduling policy'];")"
    test "$expected_sched_policy" = "$result_sched_policy"
}

check_thread_sched_cpu_affinity()
{
    local thread_nr=$1
    local expected_cpu_affinity=$2
    local result_file=$3
    local result_cpu_affinity="$(python -c "import yaml; y = yaml.load(open('$result_file')); print y['threads'][$thread_nr]['cpu affinity'];")"
    test "$expected_cpu_affinity" = "$result_cpu_affinity"
}

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
    '$DABBA_PATH'/dabba thread list > result
"

test_expect_success PYTHON_YAML "Check thread number" "
    check_thread_nr 1 result
"

test_expect_success PYTHON_YAML "Check thread ID" "
    check_thread_id 0 result
"

test_expect_success PYTHON_YAML "Check thread type" "
    check_thread_type 0 capture result
"

test_expect_success PYTHON_YAML "Check thread default scheduling policy" "
    check_thread_sched_policy 0 other result
"

test_expect_success PYTHON_YAML "Check thread default scheduling priority" "
    check_thread_sched_prio 0 0 result
"

test_expect_success TASKSET,PYTHON_YAML "Check thread default CPU affinity" "
    check_thread_sched_cpu_affinity 0 '$(get_default_cpu_affinity)' result
"

test_expect_success PYTHON_YAML "Stop capture thread using thread output" "
    '$DABBA_PATH'/dabba thread list > result &&
    '$DABBA_PATH'/dabba capture stop --id `get_thread_id 0 result` &&
    '$DABBA_PATH'/dabba thread list > after &&
    test_must_fail grep -wq `get_thread_id 0 result` after
"

test_expect_success "Cleanup: Stop dabbad" "
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et
