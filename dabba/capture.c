/**
 * \file capture.c
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

dabba-capture - Manage capture threads

=head1 SYNOPSIS

dabba capture <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage capture threads on the system
and to list information about currently running captures.

=head1 COMMANDS

=over

=item list

Fetch and print information about currently running captures.
The output is formatted in YAML.

=item start

Start a new capture.

=item stop

Stop a running capture.

=back

=head1 OPTIONS

=over

=item --interface <name>

Precise on which interface the capture must run.
Use "dabba interface list" to see the list of supported interfaces.
(the special interface "any" captures on all up and running interfaces).

=item --pcap <path>

Write all captured traffic in pcap file at <path>.

=item --frame-number <number>

Configure the packet mmap area to contain <number> of frames.
This number must be a power of two. The default value is 32 frames.
The lowest frame number value is 8.

=item --id <thread-id>

Reference a capture by its unique thread id.
The capture id can be fetched using "dabba capture list".

=back

=head1 EXAMPLES

=over

=item dabba capture list

Output information about all running captures

=item dabba capture start --interface eth0 --pcap eth0.pcap --frame-number 128

Starts a capture listening on eth0 and dumps all data in
the pcap file "eth0.pcap". The allocated packet mmap area can
contain 128 frames.

=item dabba capture stop --id 123456789

Stop running capture which has the id "123456789"

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
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>

#include <libdabba/macros.h>
#include <libdabba/packet_mmap.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/rpc.h>
#include <dabba/thread.h>
#include <dabbad/dabbad.h>

#define DEFAULT_CAPTURE_FRAME_NUMBER 32

enum capture_start_option {
	OPT_CAPTURE_INTERFACE,
	OPT_CAPTURE_PCAP,
	OPT_CAPTURE_FRAME_NUMBER
};

enum capture_stop_option {
	OPT_CAPTURE_ID
};

static struct option *capture_start_options_get(void)
{
	static struct option capture_start_option[] = {
		{"interface", required_argument, NULL, OPT_CAPTURE_INTERFACE},
		{"pcap", required_argument, NULL, OPT_CAPTURE_PCAP},
		{"frame-number", required_argument, NULL,
		 OPT_CAPTURE_FRAME_NUMBER},
		{NULL, 0, NULL, 0},
	};

	return capture_start_option;
}

static int prepare_capture_start_query(int argc, char **argv,
				       Dabba__Capture * settingsp)
{
	int ret, rc = 0;

	assert(settingsp);

	/* Assume conservative values for now */
	settingsp->has_frame_nr = settingsp->has_frame_size = 1;
	settingsp->frame_size = PACKET_MMAP_ETH_FRAME_LEN;
	settingsp->frame_nr = DEFAULT_CAPTURE_FRAME_NUMBER;

	while ((ret =
		getopt_long_only(argc, argv, "", capture_start_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_CAPTURE_INTERFACE:
			settingsp->interface = optarg;
			break;

		case OPT_CAPTURE_PCAP:
			settingsp->pcap = optarg;
			break;
		case OPT_CAPTURE_FRAME_NUMBER:
			settingsp->frame_nr = strtoull(optarg, NULL, 10);
			break;
			/*TODO make frame size configurable */
		default:
			show_usage(capture_start_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

static void capture_dummy_print(const Dabba__Dummy * result, void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(result);
	assert(closure_data);

	*status = 1;
}

static void display_capture_list_header(void)
{
	printf("---\n");
	printf("  captures:\n");
}

static void capture_list_print(const Dabba__CaptureList *
			       result, void *closure_data)
{
	const Dabba__Capture *capture;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	display_capture_list_header();

	for (a = 0; result && a < result->n_list; a++) {
		capture = result->list[a];
		printf("    - id: %" PRIu64 "\n", (uint64_t) capture->id->id);
		printf("      packet mmap size: %" PRIu64 "\n",
		       capture->frame_nr * capture->frame_size);
		printf("      frame number: %" PRIu64 "\n", capture->frame_nr);
		printf("      pcap: %s\n", capture->pcap);
		printf("      interface: %s\n", capture->interface);
	}

	*status = 1;
}

/**
 * \brief Prepare a command to start a capture.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_start(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__Capture capture_settings = DABBA__CAPTURE__INIT;

	assert(argc >= 0);
	assert(argv);

	prepare_capture_start_query(argc, (char **)argv, &capture_settings);

	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	/* TODO Print create capture thread id ? */
	dabba__dabba_service__capture_start(service, &capture_settings,
					    capture_dummy_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare a command to list current captures.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_settings_list(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;

	assert(argc >= 0);
	assert(argv);

	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	dabba__dabba_service__capture_get(service, &id_list, capture_list_print,
					  &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static struct option *capture_stop_options_get(void)
{
	static struct option capture_stop_option[] = {
		{"id", required_argument, NULL, OPT_CAPTURE_ID},
		{NULL, 0, NULL, 0},
	};

	return capture_stop_option;
}

static int prepare_capture_stop_query(int argc, char **argv,
				      Dabba__ThreadId * idp)
{
	int ret, rc = 0;

	assert(idp);

	while ((ret =
		getopt_long_only(argc, argv, "", capture_stop_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_CAPTURE_ID:
			idp->id = strtoull(optarg, NULL, 10);
			break;
		default:
			show_usage(capture_stop_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Prepare a command to stop a active capture.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_stop(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__ThreadId id = DABBA__THREAD_ID__INIT;

	assert(argc >= 0);
	assert(argv);

	prepare_capture_stop_query(argc, (char **)argv, &id);

	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	/* TODO Print create capture thread id ? */
	dabba__dabba_service__capture_stop(service, &id,
					   capture_dummy_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Parse which capture sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the capture sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_capture(int argc, const char **argv)
{
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct capture_commands[] = {
		{"list", cmd_capture_settings_list},
		{"start", cmd_capture_start},
		{"stop", cmd_capture_stop},
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(capture_commands); i++)
		if (!strcmp(capture_commands[i].cmd, cmd))
			return run_builtin(&capture_commands[i], argc, argv);

	return ENOSYS;
}
