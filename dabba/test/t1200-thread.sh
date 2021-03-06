#!/bin/sh
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

test_description='Test dabba thread command'

. ./dabba-test-lib.sh

pidfile=$(mktemppid)

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
    dabbad --daemonize --pidfile '$pidfile'
"

test_expect_success "Setup: Start a basic capture on loopback" "
    dabba capture start --interface any --pcap test.pcap --frame-number 8
"

test_expect_success "Setup: Start a basic replay on loopback" "
    dabba replay start --interface lo --pcap test.pcap --frame-number 8
"

test_expect_success "Check 'dabba thread' help output" "
    dabba help thread | cat <<EOF
    q
    EOF &&
    dabba thread --help | cat <<EOF
    q
    EOF
"

test_expect_success "Fetch running thread information" "
    dabba thread get settings > result
"

test_expect_success PYTHON_YAML "Parse thread YAML output" "
    yaml2dict result > parsed
"

for i in $(seq 0 1)
do
    case $i in
        0) thread_type="capture";;
        1) thread_type="replay";;
    esac

    test_expect_success PYTHON_YAML "Query thread YAML output" "
        echo '$thread_type' > expect_type &&
        echo 'other' > expect_scheduling_policy &&
        echo '0' > expect_scheduling_priority &&
        echo '$default_cpu_affinity' > expect_cpu_affinity &&
        dictkeys2values threads $i id < parsed > result_id.$i &&
        dictkeys2values threads $i type < parsed > result_type &&
        dictkeys2values threads $i 'scheduling policy' < parsed > result_scheduling_policy &&
        dictkeys2values threads $i 'scheduling priority' < parsed > result_scheduling_priority &&
        dictkeys2values threads $i 'cpu affinity' < parsed > result_cpu_affinity
    "

    test_expect_success PYTHON_YAML "Check thread number" "
        check_thread_nr 2 result
    "

    test_expect_success PYTHON_YAML "Check thread ID" "
        grep -wq -E '^[0-9]+' 'result_id.$i'
    "

    test_expect_success PYTHON_YAML "Check thread type" "
        test_cmp expect_type result_type
    "

    test_expect_success PYTHON_YAML "Check thread default scheduling policy" "
        test_cmp expect_scheduling_policy result_scheduling_policy
    "

    test_expect_success PYTHON_YAML "Check thread default scheduling priority" "
        test_cmp expect_scheduling_priority result_scheduling_priority
    "

    test_expect_success TASKSET,PYTHON_YAML "Check thread default CPU affinity" "
        test_cmp expect_cpu_affinity result_cpu_affinity
    "
done

for policy in fifo rr other
do
        min_prio=$(get_default_sched_prio_min $policy)
        max_prio=$(get_default_sched_prio_max $policy)

        for priority in $min_prio $max_prio
        do
                test_expect_success PYTHON_YAML "Modify capture thread scheduling policy ($policy:$priority)" "
                    dabba thread modify --sched-policy '$policy' --sched-prio '$priority' --id '$(cat result_id.0)' &&
                    dabba thread get settings > result
                "

                test_expect_success PYTHON_YAML "Parse thread YAML output" "
                    yaml2dict result > parsed
                "

                test_expect_success PYTHON_YAML "Query thread YAML output" "
                    echo '$policy' > expect_scheduling_policy &&
                    echo '$priority' > expect_scheduling_priority &&
                    dictkeys2values threads 0 'scheduling policy' < parsed > result_scheduling_policy &&
                    dictkeys2values threads 0 'scheduling priority' < parsed > result_scheduling_priority
                "

                test_expect_success PYTHON_YAML "Check new capture thread scheduling policy" "
                    test_cmp expect_scheduling_policy result_scheduling_policy
                "

                test_expect_success PYTHON_YAML "Check new capture thread scheduling priority" "
                    test_cmp expect_scheduling_priority result_scheduling_priority
                "
        done

        for priority in $(($min_prio-1)) $(($max_prio+1))
        do
                # Put back to 'test_must_fail' when proper error reporting is done
                test_expect_success PYTHON_YAML "Do not modify capture thread out-of-range scheduling policy ($policy:$priority)" "
                    test_might_fail dabba thread modify --sched-policy '$policy' --sched-prio '$priority' --id '$(cat result_id.0)' &&
                    dabba thread get settings > result
                "

                test_expect_success PYTHON_YAML "Parse thread YAML output" "
                    yaml2dict result > parsed
                "

                test_expect_success PYTHON_YAML "Query thread YAML output" "
                    dictkeys2values threads 0 'scheduling policy' < parsed > result_scheduling_policy &&
                    dictkeys2values threads 0 'scheduling priority' < parsed > result_scheduling_priority
                "

                test_expect_success PYTHON_YAML "Check that capture thread scheduling policy is unchanged" "
                    test_cmp expect_scheduling_policy result_scheduling_policy
                "

                test_expect_success PYTHON_YAML "Check that capture thread scheduling priority is unchanged" "
                    test_cmp expect_scheduling_priority result_scheduling_priority
                "
        done
done

for cpu_affinity in 0 $default_cpu_affinity
do
        test_expect_success PYTHON_YAML "Modify capture thread CPU affinity (run on CPU $cpu_affinity)" "
            dabba thread modify --cpu-affinity '$cpu_affinity' --id '$(cat result_id.0)' &&
            dabba thread get settings > result
        "

        test_expect_success PYTHON_YAML "Parse thread YAML output" "
            yaml2dict result > parsed
        "

        test_expect_success PYTHON_YAML "Query thread YAML output" "
            echo '$cpu_affinity' > expect_cpu_affinity &&
            dictkeys2values threads 0 'cpu affinity' < parsed > result_cpu_affinity
        "

        test_expect_success TASKSET,PYTHON_YAML "Check thread modified CPU affinity" "
            test_cmp expect_cpu_affinity result_cpu_affinity
        "
done

test_expect_success "Stop all running captures" "
    dabba capture stop-all &&
    dabba thread get settings > after
"

test_expect_success PYTHON_YAML "Check if the capture thread is still present" "
    test_must_fail grep -wq '$(cat result_id.0)' after
"

test_expect_success "Cleanup: Stop dabbad" "
    kill $(cat "$pidfile")
"

test_done

# vim: ft=sh:tabstop=4:et
