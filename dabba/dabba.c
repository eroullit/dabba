/**
 * \file dabba.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


/*

=head1 NAME

dabba - Send queries to dabbad and process its replies.

=head1 SYNOPSIS

dabba <command> <arguments> [--help]

=head1 DESCRIPTION

dabba is here to generate RPC queries to dabbad and output its
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

=item Copyright (C) 2013 Emmanuel Roullit.

=item License MIT: <www.opensource.org/licenses/MIT>

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
#include <dabba/replay.h>

/**
 * \internal
 * \brief run command function and pass it \c argc / \c argv
 * \param[in]           p	        Pointer to command structure
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, else on failure.
 * \note on success, \c stdout pipe/socket is checked, flushed and closed.
 */

static int run_builtin(const struct cmd_struct *p, int argc, const char **argv)
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

int cmd_run_action(const struct cmd_struct *cmd, const size_t cmd_len,
		   int argc, const char **argv)
{
	const char *cmd_str = argv[0];
	size_t a;

	for (a = 0; a < cmd_len; a++) {
		if (strcmp(cmd[a].cmd, cmd_str))
			continue;

		return run_builtin(&cmd[a], --argc, ++argv);
	}

	help_unknown_cmd(cmd_str);

	return ENOSYS;
}

/**
 * \brief run command function and pass it \c argc / \c argv
 * \brief also check if \c argv contains help parameters
 * \param[in]           p	        Pointer to command structure
 * \param[in]           cmd_len	        Number of valid command passed
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, else on failure, \c ENOSYS if the command is unknown.
 * \note on success, \c stdout pipe/socket is checked, flushed and closed.
 */

int cmd_run_command(const struct cmd_struct *cmd, const size_t cmd_len,
		    int argc, const char **argv)
{
	if (argc == 0 || argv[0] == NULL || !strcmp(argv[0], "--help"))
		argv[0] = "help";

	if (!strcmp(argv[0], "--version"))
		argv[0] = "version";

	/* Turn "dabba cmd --help" into "dabba help cmd" */
	if (argc > 1 && !strcmp(argv[1], "--help")) {
		argv[1] = argv[0];
		argv[0] = "help";
	}

	return cmd_run_action(cmd, cmd_len, argc, argv);
}

/**
 * \brief start function. Pass on \c argc / \c argv to commands
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, else on failure.
 */

int main(int argc, const char **argv)
{
	static const struct cmd_struct commands[] = {
		{"interface", cmd_interface},
		{"thread", cmd_thread},
		{"capture", cmd_capture},
		{"replay", cmd_replay},
		{"version", cmd_version},
		{"help", cmd_help}
	};

	return cmd_run_command(commands, ARRAY_SIZE(commands), --argc, ++argv);
}
