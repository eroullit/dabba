test_description='Test dabba list command'

. ./sharness.sh

DABBAD_PATH="$TEST_DIRECTORY/../../dabbad"
DABBA_PATH="$TEST_DIRECTORY/../../dabba"

test_expect_failure 'invoke dabba list w/o dabbad' "$DABBA_PATH/dabba list"
test_expect_success 'invoke dabba list with dabbad' "
    $DABBAD_PATH/dabbad --daemonize &&
    $DABBA_PATH/dabba list &&
    killall dabbad
"

test_done

# vim: ft=sh:tabstop=4:et