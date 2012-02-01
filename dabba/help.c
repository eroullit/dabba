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
#include <string.h>
#include <assert.h>
#include <dabbacore/macros.h>

const char dabba_usage_string[] = "dabba [--help] <command> [<args>]\n";
const char dabba_more_info_string[] =
    "See 'dabba help <command>' for more info on a specific command.";

struct cmdname_help {
	char name[16];
	char help[80];
};

static inline void mput_char(char c, uint32_t num)
{
	while (num--)
		putchar(c);
}

void list_common_cmds_help(void)
{
	size_t i, longest = 0;

	static struct cmdname_help common_cmds[] = {
		{"list", "list available network interfaces"}
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

int cmd_help(int argc, const char **argv)
{
	assert(argc >= 0);
	assert(argv);

	if (!argv[0]) {
		printf("usage: %s\n\n", dabba_usage_string);
		list_common_cmds_help();
		printf("\n%s\n", dabba_more_info_string);
		return 0;
	}

	list_common_cmds_help();

	return 0;
}