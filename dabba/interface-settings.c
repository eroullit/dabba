/**
 * \file interface-settings.c
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
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <dabbad/dabbad.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

extern const char *ethtool_port_str_get(const uint8_t port);

static void display_interface_settings_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_settings_list_print(const Dabba__InterfaceSettingsList *
					  result, void *closure_data)
{
	Dabba__InterfaceSettings *settingsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_settings_header();

	for (a = 0; result && a < result->n_list; a++) {
		settingsp = result->list[a];
		printf("    - name: %s\n", settingsp->id->name);
		printf("      settings:\n");
		printf("        speed: %u\n", settingsp->speed);
		printf("        duplex: %s\n",
		       settingsp->duplex ? "full" : "half");
		printf("        autoneg: %s\n", print_tf(settingsp->autoneg));
		printf("        mtu: %u\n", settingsp->mtu);
		printf("        tx qlen: %u\n", settingsp->tx_qlen);
		printf("        port: %s\n",
		       ethtool_port_str_get(settingsp->port));
		printf("        max rx packet: %u\n", settingsp->maxrxpkt);
		printf("        max tx packet: %u\n", settingsp->maxtxpkt);
	}

	*status = 1;
}

static int prepare_interface_settings_query(int argc, char **argv,
					    char **server_name,
					    Dabba__InterfaceIdList * list)
{
	enum interface_pause_option {
		OPT_SERVER_ID,
		OPT_INTERFACE_ID,
		OPT_HELP,
	};
	const struct option interface_pause_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"server", required_argument, NULL, OPT_SERVER_ID},
		{NULL, 0, NULL, 0},
	};
	int ret, rc = 0;
	Dabba__InterfaceId **idpp;

	assert(list);

	while ((ret =
		getopt_long_only(argc, argv, "", interface_pause_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_SERVER_ID:
			*server_name = optarg;
			break;
		case OPT_INTERFACE_ID:
			idpp =
			    realloc(list->list,
				    sizeof(*list->list) * (list->n_list + 1));

			if (!idpp)
				return ENOMEM;

			list->list = idpp;
			list->list[list->n_list] =
			    malloc(sizeof(*list->list[list->n_list]));

			if (!list->list[list->n_list])
				return ENOMEM;

			dabba__interface_id__init(list->list[list->n_list]);

			list->list[list->n_list]->name = optarg;
			list->n_list++;

			break;
		case OPT_HELP:
		default:
			show_usage(interface_pause_option);
			rc = -1;
			break;
		}
	}

	return rc;
}

int cmd_interface_settings(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	char *server_name = NULL;
	size_t a;
	int rc;

	assert(argc >= 0);
	assert(argv);

	rc = prepare_interface_settings_query(argc, (char **)argv, &server_name,
					      &id_list);

	if (rc)
		goto out;

	service = dabba_rpc_client_connect(server_name);

	dabba__dabba_service__interface_settings_get(service, &id_list,
						     interface_settings_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

 out:
	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);

	return rc;
}
