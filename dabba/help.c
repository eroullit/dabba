/**
 * \file help.c
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include <limits.h>
#include <libdabba/macros.h>

static const char dabba_usage_string[] =
    "dabba [--help] [--version] <command> [<subcommand>] <action> [<args>]\n";
static const char dabba_more_info_string[] =
    "See 'dabba help <command> [<subcommand>]' for more specific information.";

struct cmdname_help {
	char name[16];
	char help[80];
};

/**
 * \brief print option list to \c stdout
 * \param[in]           opt	        Pointer to option array
 */

void show_usage(const struct option *opt)
{
	assert(opt);

	printf("%s", dabba_usage_string);

	while (opt->name != NULL) {
		printf("  --%s", opt->name);
		if (opt->has_arg == required_argument)
			printf(" <arg>\n");
		else if (opt->has_arg == optional_argument)
			printf(" [arg]\n");
		else
			printf("\n");
		opt++;
	}
}

/**
 * \internal
 * \brief add some indenting on \c stdout
 * \param[in]           c	        Character to indent
 * \param[in]           num	        Indent level
 */

static inline void mput_char(const char c, uint32_t num)
{
	while (num--)
		putchar(c);
}

/**
 * \brief print on \c stdout that the invoked command is unknown
 * \param[in]           cmd	        Pointer to invoked command string
 */

void help_unknown_cmd(const char *const cmd)
{
	assert(cmd);
	printf("'%s' is not a dabba command. See 'dabba --help'.\n", cmd);
}

/**
 * \brief list on \c stdout the commonly used commands.
 */

static void list_common_cmds_help(void)
{
	size_t i, longest = 0;

	static struct cmdname_help common_cmds[] = {
		{"interface", "perform an interface related command"},
		{"thread", "perform a thread related command"},
		{"capture", "capture live traffic from an interface"}
	};

	for (i = 0; i < ARRAY_SIZE(common_cmds); i++) {
		if (longest < strlen(common_cmds[i].name))
			longest = strlen(common_cmds[i].name);
	}

	printf("The most commonly used dabba commands are:\n");
	for (i = 0; i < ARRAY_SIZE(common_cmds); i++) {
		printf("   %s   ", common_cmds[i].name);
		mput_char(' ', longest - strlen(common_cmds[i].name));
		printf("%s\n", common_cmds[i].help);
	}
}

/**
 * \brief Print help
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return Returns 0 when the help could be printed, \c errno otherwise
 *
 * This function prints the basic dabba usage help message.
 * Furthermore, it lists all available command.
 */

int cmd_help(int argc, const char **argv)
{
	char help_name[NAME_MAX];
	int rc = 0, a;
	size_t offset;

	if (argc < 1 || !argv[0]) {
		printf("usage: %s\n\n", dabba_usage_string);
		list_common_cmds_help();
		printf("\n%s\n", dabba_more_info_string);
	} else {
		offset = snprintf(help_name, sizeof(help_name), "dabba");

		for (a = 0; a < argc; a++)
			offset +=
			    snprintf(&help_name[offset],
				     sizeof(help_name) - offset, "-%s",
				     argv[a]);

		rc = execlp("man", "man", help_name, NULL);
	}

	return rc ? errno : rc;
}

/**
 * \brief Print tool version number to \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return Always returns zero.
 */

int cmd_version(int argc, const char **argv)
{
	(void)argc;
	(void)argv;

	printf("dabba version %s\n", DABBA_VERSION);
	return 0;
}
