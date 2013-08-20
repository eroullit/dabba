#!/bin/sh
#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

test_description='Test dabba help output'

. ./dabba-test-lib.sh

cat > expect <<EOF
usage: dabba [--help] [--version] <command> [<subcommand>] <action> [<args>]


The most commonly used dabba commands are:
   interface   perform an interface related command
   thread      perform a thread related command
   capture     capture live traffic from an interface
   replay      replay traffic from a pcap file

See 'dabba help <command> [<subcommand>]' for more specific information.
EOF

test_expect_success "Check dabba help output" "
    dabba --help > result &&
    test_cmp expect result
"

test_done

# vim: ft=sh:tabstop=4:et
