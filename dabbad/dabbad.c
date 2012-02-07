/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include <dabbad/ipc.h>
#include <dabbad/help.h>

enum dabbad_opts {
	OPT_DAEMONIZE,
	OPT_HELP
};

const struct option *dabbad_options_get(void)
{
	static const struct option dabbad_long_options[] = {
		{"daemonize", no_argument, NULL, OPT_DAEMONIZE},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0}
	};

	return (dabbad_long_options);
}

int main(int argc, char **argv)
{
	int opt, opt_idx;
	int daemonize = 0;

	assert(argc);
	assert(argv);

	while ((opt =
		getopt_long_only(argc, argv, "", dabbad_options_get(),
				 &opt_idx)) != EOF) {
		switch (opt) {
		case OPT_DAEMONIZE:
			daemonize = 1;
			break;
		case OPT_HELP:
		default:
			show_usage(dabbad_options_get());
			return EXIT_FAILURE;
			break;

		}
	}

	if (daemonize) {
		if (daemon(0, 0)) {
			perror("Could not daemonize process");
			return errno;
		}
	}

	dabbad_ipc_msg_init();

	return dabbad_ipc_msg_poll();
}
