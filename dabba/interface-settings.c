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
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/cli.h>
#include <dabba/help.h>

extern const char *port2str(const uint8_t port);

static void interface_settings_list_print(const Dabba__InterfaceSettingsList *
					  result, void *closure_data)
{
	Dabba__InterfaceSettings *settingsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		settingsp = result->list[a];
		printf("    ");
		__rpc_error_code_print(settingsp->status->code);
		printf("    - name: %s\n", settingsp->id->name);
		printf("      settings:\n");
		printf("        speed: %u\n", settingsp->speed);
		printf("        duplex: %s\n",
		       settingsp->duplex ? "full" : "half");
		printf("        autoneg: %s\n", print_tf(settingsp->autoneg));
		printf("        mtu: %u\n", settingsp->mtu);
		printf("        tx qlen: %u\n", settingsp->tx_qlen);
		printf("        port: %s\n", port2str(settingsp->port));
		printf("        max rx packet: %u\n", settingsp->maxrxpkt);
		printf("        max tx packet: %u\n", settingsp->maxtxpkt);
	}

	*status = 1;
}

int rpc_interface_settings_get(ProtobufCService * service,
			       const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_settings_get(service, id_list,
						     interface_settings_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int rpc_interface_settings_modify(ProtobufCService * service,
					 const Dabba__InterfaceSettings *
					 settingsp)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(settingsp);

	dabba__dabba_service__interface_settings_modify(service, settingsp,
							rpc_error_code_print,
							&is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int cmd_interface_settings_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_SPEED,
		OPT_INTERFACE_DUPLEX,
		OPT_INTERFACE_AUTONEG,
		OPT_INTERFACE_MTU,
		OPT_INTERFACE_TXQLEN,
		OPT_INTERFACE_PORT,
		OPT_INTERFACE_MAX_RXPKT,
		OPT_INTERFACE_MAX_TXPKT,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"speed", required_argument, NULL, OPT_INTERFACE_SPEED},
		{"duplex", required_argument, NULL, OPT_INTERFACE_DUPLEX},
		{"autoneg", required_argument, NULL, OPT_INTERFACE_AUTONEG},
		{"mtu", required_argument, NULL, OPT_INTERFACE_MTU},
		{"txqlen", required_argument, NULL, OPT_INTERFACE_TXQLEN},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceSettings settings = DABBA__INTERFACE_SETTINGS__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_INTERFACE_SPEED:
			rc = str2speed(optarg, &settings.speed);

			if (rc)
				goto out;

			settings.has_speed = 1;
			break;
		case OPT_INTERFACE_DUPLEX:
			rc = str2duplex(optarg, &settings.duplex);

			if (rc)
				goto out;

			settings.has_duplex = 1;
			break;
		case OPT_INTERFACE_AUTONEG:
			rc = str2bool(optarg, &settings.autoneg);

			if (rc)
				goto out;

			settings.has_autoneg = 1;
			break;
		case OPT_INTERFACE_MTU:
			settings.mtu = strtoul(optarg, NULL, 10);
			settings.has_mtu = 1;
			break;
		case OPT_INTERFACE_TXQLEN:
			settings.tx_qlen = strtoul(optarg, NULL, 10);
			settings.has_tx_qlen = 1;
			break;
		case OPT_INTERFACE_MAX_RXPKT:
			settings.maxrxpkt = strtoul(optarg, NULL, 10);
			settings.has_maxrxpkt = 1;
			break;
		case OPT_INTERFACE_MAX_TXPKT:
			settings.maxtxpkt = strtoul(optarg, NULL, 10);
			settings.has_maxtxpkt = 1;
			break;
		case OPT_TCP:
			server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
			server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_LOCAL:
			server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
			server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_INTERFACE_ID:
			settings.id = malloc(sizeof(*settings.id));

			if (!settings.id)
				return ENOMEM;

			dabba__interface_id__init(settings.id);
			settings.id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	settings.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_settings_modify(service, &settings);
	else
		rc = EINVAL;
 out:
	free(settings.id);
	return rc;
}
