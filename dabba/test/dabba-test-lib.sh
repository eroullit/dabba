#!/bin/sh
#
# Copyright (C) 2013    Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

. ./sharness.sh

DABBAD_PATH="$SHARNESS_TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$SHARNESS_TEST_DIRECTORY/../../dabba"
TOOLS_PATH="$SHARNESS_TEST_DIRECTORY/tools"
PATH="$DABBA_PATH:$DABBAD_PATH:$TOOLS_PATH:$PATH"

PYTHON_PATH="$(command -v python)"
ETHTOOL_PATH="$(command -v ethtool)"

"$PYTHON_PATH" -c "import yaml" > /dev/null 2>&1 && test_set_prereq PYTHON_YAML
taskset -h > /dev/null 2>&1 && test_set_prereq TASKSET
"$ETHTOOL_PATH" -h > /dev/null 2>&1 && test_set_prereq ETHTOOL
test -n "$TEST_DEV" && test_set_prereq TEST_DEV

mktemppid()
{
    mktemp dabbad.pid.XXXXXXXXXXXX
}

yaml2dict()
{
    "$PYTHON_PATH" "$SHARNESS_TEST_DIRECTORY"/yaml2dict.py "$@"
}

dictkeys2values()
{
    "$PYTHON_PATH" "$SHARNESS_TEST_DIRECTORY"/dictkeys2values.py "$@"
}

yaml_number_of_interface_get()
{
    local result_file=$1
    "$PYTHON_PATH" -c "import yaml; y = yaml.load(open('$result_file')); print len(y['interfaces']);"
}

number_of_interface_get()
{
    sed '1,2d' /proc/net/dev | wc -l
}

sys_class_net_get()
{
    local dev="$(cat "$1")"
    local param="$2"
    local rc

    cat "/sys/class/net/$dev/$param" > tmp_set 2> /dev/null
    rc=$?

    (test -s tmp_set && test $rc = 0 && cat tmp_set) || echo "0"
}

# This function prints all text between a beginning and end regexes.
# beginning and end regexes are not printed
# $1 begin regex
# $2 end regex
# $3 file to read
range_print() {
    local begin="$1"
    local end="$2"
    local file="$3"

    awk '/'"$end"'/{e=0}/'"$begin"'/{gsub("'"$begin"'","",$0);e=1}{if(e==1){print}}' "$file"
}

# This function prints 'True' or 'False', if ethtool feature returns 'on' or 'off'
# $1 begin regex
# $2 ethtool output file to read
ethtool_status_parse() {
    local pattern="$1"
    local ethtool_output="$2"
    local status=""

    status="$(grep -i "$pattern" "$ethtool_output" | awk '{print $NF}')"

    test "$status" = "on" && echo "True" || echo "False"
}

# vim: ft=sh:tabstop=4:et
