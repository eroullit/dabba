/**
 * \file dabba.c
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

dabba - Send queries to dabbad and process its replies.

=head1 SYNOPSIS

dabba <command> <arguments> [--help]

=head1 DESCRIPTION

dabba is here to generate IPC queries to dabbad and output its
replies formatted in a YAML output

=head1 OPTIONS

=over

=item --help

Prints help output and the currently supported commands.

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
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

#include <libdabba/macros.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/thread.h>
#include <dabba/interface.h>
#include <dabba/capture.h>

int run_builtin(struct cmd_struct *p, int argc, const char **argv)
{
	int status;
	struct stat st;

	assert(p);

	status = p->fn(argc, argv);
	if (status)
		return status;

	/* Somebody closed stdout? */
	if (fstat(fileno(stdout), &st))
		return 0;
	/* Ignore write errors for pipes and sockets.. */
	if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
		return 0;

	/* Check for ENOSPC and EIO errors.. */
	if (fflush(stdout)) {
		perror("write failure on standard output");
		return (errno);
	}

	if (ferror(stdout)) {
		perror("unknown write failure on standard output");
		return (1);
	}

	if (fclose(stdout)) {
		perror("close failed on standard output");
		return (errno);
	}

	return 0;
}

static int handle_internal_command(int argc, const char **argv)
{
	size_t i;
	const char *cmd = argv[0];
	static struct cmd_struct commands[] = {
		{"interface", cmd_interface},
		{"thread", cmd_thread},
		{"capture", cmd_capture},
		{"version", cmd_version},
		{"help", cmd_help}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--version"))
		cmd = "version";

	/* Turn "dabba cmd --help" into "dabba help cmd" */
	if (argc > 1 && !strcmp(argv[1], "--help")) {
		argv[1] = argv[0];
		argv[0] = cmd = "help";
	}

	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		struct cmd_struct *p = commands + i;
		if (strcmp(p->cmd, cmd))
			continue;

		argc--;
		argv++;

		return (run_builtin(p, argc, argv));
	}

	help_unknown_cmd(cmd);

	return ENOSYS;
}

int main(int argc, const char **argv)
{
	assert(argc);
	assert(argv);

	argc--;
	argv++;

	return (handle_internal_command(argc, argv));
}
