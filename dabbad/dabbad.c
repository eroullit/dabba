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

dabbad [--daemonize] [--pidfile <path>] [--tcp[=<port>]] [--local[=<path>]] [--help]

=head1 DESCRIPTION

dabbad (dabba daemon) is here to manage multithreaded network captures.
It listens a RPC interface and waits for commands to execute.

=head1 OPTIONS

=over

=item --daemonize

Detach process from the terminal and continue to run in the background.
Redirects stdin, stdout and stderr to /dev/null

=item --pidfile <path>

Creates a file where the pid is written

=item --tcp[=<port>]

Open a TCP socket to receive/transmit RPC messages (By default: 55994)

=item --local[=<path>]

Open a Unix domain socket to receive/transmit RPC messages (By default: /tmp/dabba)
This socket type is used by default

=back

=head1 EXAMPLES

=over

=item dabbad

Start dabbad in a classic process mode. It allows the user to
see debug messages.

=item dabbad --daemonize

Start dabbad as a daemon.

=item dabbad --pidfile /tmp/dabba.pid

Tell dabbad to write its pid in a specific pidfile.

=item dabbad --tcp=12345

Start a dabbad instance listening to messages from TCP port 12345

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

=item Copyright (c) 2012 Emmanuel Roullit.

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
#include <signal.h>

#include <dabbad/rpc.h>
#include <dabbad/misc.h>
#include <dabbad/help.h>

struct dabbad_config {
	const char *pidfile;
	const char *server_id;
	ProtobufC_RPC_AddressType server_type;
};

static struct dabbad_config conf = {
	.pidfile = DEFAULT_PIDPATH,.server_id =
	    DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME,.server_type =
	    PROTOBUF_C_RPC_ADDRESS_LOCAL
};

static void exit_cleanup(int arg)
{
	unlink(conf.pidfile);

	if (conf.server_type == PROTOBUF_C_RPC_ADDRESS_LOCAL)
		unlink(conf.server_id);

	exit(arg);
}

/**
 * \brief Dabbad entry point
 * \param[in]       argc	        Argument counter
 * \param[in]       argv	        Argument vector
 * \return 0 on success, else on failure
 * 
 * This function is dabbad entry point.
 * It parses given arguments, configure the daemon accordingly and start
 * RPC message polling.
 */

int main(int argc, char **argv)
{
	enum dabbad_opts {
		OPT_DAEMONIZE,
		OPT_PIDFILE,
		OPT_TCP,
		OPT_LOCAL,
		OPT_VERSION,
		OPT_HELP
	};

	static const struct option dabbad_long_options[] = {
		{"daemonize", no_argument, NULL, OPT_DAEMONIZE},
		{"pidfile", required_argument, NULL, OPT_PIDFILE},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"version", no_argument, NULL, OPT_VERSION},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0}
	};

	int opt, opt_idx, rc = 0;
	int daemonize = 0;

	assert(argc);
	assert(argv);

	while ((opt =
		getopt_long_only(argc, argv, "", dabbad_long_options,
				 &opt_idx)) != EOF) {
		switch (opt) {
		case OPT_DAEMONIZE:
			daemonize = 1;
			break;
		case OPT_PIDFILE:
			conf.pidfile = optarg;
			break;
		case OPT_VERSION:
			print_version();
			return EXIT_SUCCESS;
			break;
		case OPT_TCP:
			conf.server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
			conf.server_id = DABBA_RPC_DEFAULT_PORT;

			if (optarg)
				conf.server_id = optarg;
			break;
		case OPT_LOCAL:
			conf.server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
			conf.server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;

			if (optarg)
				conf.server_id = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(dabbad_long_options);
			return EXIT_SUCCESS;
			break;

		}
	}

	if (daemonize) {
		if (daemon(-1, 0)) {
			perror("Could not daemonize process");
			return errno;
		}
	}

	signal(SIGTERM, exit_cleanup);
	signal(SIGINT, exit_cleanup);
	signal(SIGQUIT, exit_cleanup);

	rc = create_pidfile(conf.pidfile);

	return rc ? rc : dabbad_rpc_msg_poll(conf.server_id, conf.server_type);
}
