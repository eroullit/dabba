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
