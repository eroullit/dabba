/**
 * \file help.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <stdio.h>
#include <assert.h>
#include <getopt.h>

static const char _usage[] =
    "usage: dabbad [<args>]\n\n" "The available options are:\n";

/**
 * \brief Show dabbad usage options
 * \param[in]       opt	        Pointer to option structure array
 */

void show_usage(const struct option *opt)
{
	assert(opt);

	printf("%s", _usage);

	if (opt != NULL) {
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
}

/**
 * \brief Show current \c dabbad version on \c stdout
 * \return Always returns 0.
 */

int print_version(void)
{
	printf("dabbad version %s\n", DABBA_VERSION);
	return 0;
}
