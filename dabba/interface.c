/**
 * \file interface.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2012
 * \date 2012
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 *
 */

/* __LICENSE_HEADER_END__ */

/*

=head1 NAME

dabba-interface - Manage network interface

=head1 SYNOPSIS

dabba interface <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the available network interfaces.

=head1 COMMANDS

=over

=item list

Fetch and print information about currenty supported interfaces.
The output is formatted in YAML.

=back

=head1 EXAMPLES

=over

=item dabba interface list

Output the list of network interface which can be used by dabba.

=back

=head1 AUTHOR

Written by Emmanuel Roullit <emmanuel.roullit@gmail.com>

=head1 BUGS

=over

=item Please report bugs to <https://github.com/eroullit/dabba/issues>

=item dabba project project page: <https://github.com/eroullit/dabba>

=back

=head1 COPYRIGHT

=over

=item Copyright © 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

=item This is free software: you are free to change and redistribute it.

=item There is NO WARRANTY, to the extent permitted by law.

=back

=cut

*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <libdabba/macros.h>
#include <dabba/dabba.h>
#include <dabba/ipc.h>

static void display_interface_list_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void display_interface_list(const struct dabba_ifconf *const
				   interface_msg, const size_t elem_nr)
{
	size_t a;

	assert(interface_msg);
	assert(elem_nr <= DABBA_IFCONF_MAX_SIZE);

	for (a = 0; a < elem_nr; a++) {
		printf("    - name: %s\n", interface_msg[a].name);
		printf("      up: \"%s\"\n",
		       (interface_msg[a].flags & IFF_UP) ==
		       IFF_UP ? "yes" : "no");
		printf("      running: \"%s\"\n",
		       (interface_msg[a].flags & IFF_RUNNING) ==
		       IFF_RUNNING ? "yes" : "no");
		printf("      promiscuous: \"%s\"\n",
		       (interface_msg[a].flags & IFF_PROMISC) ==
		       IFF_PROMISC ? "yes" : "no");
		printf("      loopback: \"%s\"\n",
		       (interface_msg[a].flags & IFF_LOOPBACK) ==
		       IFF_LOOPBACK ? "yes" : "no");
	}
}

/**
 * \brief Request the current supported interface list
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 *
 * This function prepares an IPC message to query the supported network
 * interfaces present on the system. Once the message is sent, it waits for the
 * dabba daemon to reply.
 */

int cmd_interface_list(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_IFCONF;

	display_interface_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_interface_list(msg.msg_body.msg.ifconf,
				       msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

	return rc;
}

/**
 * \brief Modify parametets of a supported interface
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_modify(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_IF_MODIFY;
	msg.msg_body.elem_nr = 1;

	return dabba_ipc_msg(&msg);
}

/**
 * \brief Parse which interface sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the interface sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_interface(int argc, const char **argv)
{
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct interface_commands[] = {
		{"list", cmd_interface_list},
		{"modify", cmd_interface_modify}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(interface_commands); i++) {
		if (!strcmp(interface_commands[i].cmd, cmd))
			return run_builtin(&interface_commands[i], argc, argv);
	}

	return ENOSYS;
}
