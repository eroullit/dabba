/**
 * \file dabbad.c
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

dabbad - Multithreaded packet mmap daemon

=head1 SYNOPSIS

dabbad [--daemonize][--help]

=head1 DESCRIPTION

dabbad (dabba daemon) is here to manage multithreaded network captures.
It listens an IPC interface and waits for commands to execute.

=head1 EXAMPLES

=over

=item dabbad

Start dabbad in a classic process mode. It allows the user to
see debug messages.

=item dabbad --daemonize

Start dabbad as a daemon.

=item dabbad --help

Prints help content.

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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dabbad/rpc.h>
#include <dabbad/help.h>
#include <dabbad/dabbad.h>

enum dabbad_opts {
	OPT_DAEMONIZE,
	OPT_TCP,
	OPT_LOCAL,
	OPT_VERSION,
	OPT_HELP
};

/**
 * \brief Dabbad options getter
 * \return Dabbad option data structure
 */

const struct option *dabbad_options_get(void)
{
	static const struct option dabbad_long_options[] = {
		{"daemonize", no_argument, NULL, OPT_DAEMONIZE},
		{"tcp", required_argument, NULL, OPT_TCP},
		{"local", required_argument, NULL, OPT_LOCAL},
		{"version", no_argument, NULL, OPT_VERSION},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0}
	};

	return (dabbad_long_options);
}

/**
 * \internal
 * \brief Create dabbad pidfile
 * \return 0 on success, else on failure.
 *
 * This function creates start time a file containing dabbad process id.
 */

static inline int dabbad_pidfile_create(void)
{
	int rc = EIO;
	int pidfd = -1;
	char pidstr[8] = { 0 };
	ssize_t pidstrlen = 0;

	pidfd = creat(DABBAD_PID_FILE, 0600);

	if (pidfd < 0)
		return errno;

	pidstrlen = snprintf(pidstr, sizeof(pidstr), "%i\n", getpid());

	if (write(pidfd, pidstr, pidstrlen) != pidstrlen) {
		rc = EIO;
	}

	close(pidfd);
	return rc;
}

/**
 * \brief Dabbad entry point
 * \param[in]       argc	        Argument counter
 * \param[in]       argv	        Argument vector
 * \return 0 on success, else on failure
 * 
 * This function is dabbad entry point.
 * It parses given arguments, configure the daemon accordingly and start
 * IPC message polling.
 */

int main(int argc, char **argv)
{
	int opt, opt_idx;
	int daemonize = 0;
	const char *server_id = DABBA_RPC_DEFAULT_PORT;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;

	assert(argc);
	assert(argv);

	while ((opt =
		getopt_long_only(argc, argv, "", dabbad_options_get(),
				 &opt_idx)) != EOF) {
		switch (opt) {
		case OPT_DAEMONIZE:
			daemonize = 1;
			break;
		case OPT_VERSION:
			print_version();
			return EXIT_SUCCESS;
			break;
		case OPT_TCP:
		case OPT_LOCAL:
			server_id = optarg;
			if (opt == OPT_TCP)
				server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
			else if (opt == OPT_LOCAL)
				server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
			break;
		case OPT_HELP:
		default:
			show_usage(dabbad_options_get());
			return EXIT_SUCCESS;
			break;

		}
	}

	assert(dabbad_pidfile_create());

	if (daemonize) {
		if (daemon(-1, 0)) {
			perror("Could not daemonize process");
			return errno;
		}
	}

	return dabbad_rpc_msg_poll(server_id, server_type);
}
