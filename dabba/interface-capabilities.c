/**
 * \file interface-capabilities.c
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

dabba-interface-capabilities - Manage network interface advertised capabilities

=head1 SYNOPSIS

dabba interface <command> capabilities [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the available
network interfaces capabilities.

=head1 COMMANDS

=over

=item get

Fetch and print status capabilities information about currently supported
interfaces.
The output is formatted in YAML.

=item modify

Apply new capabilities settings to a specific network interface.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --aui (true|false)

Activate or shutdown the attachment unit interface if available.

=item --tp (true|false)

Activate or shutdown the twisted pair port if available.

=item --bnc (true|false)

Activate or shutdown the BNC port if available.

=item --fibre (true|false)

Activate or shutdown the fibre port if available.

=item --mii (true|false)

Activate or shutdown the MII port if available.

=item --speed (10|100|1000|10000)

Select which advertised speed capabilities should be modified.

=item --half-duplex (true|false)

Set the interface half duplex advertised capability at a specific speed

=item --full-duplex (true|false)

Set the interface full duplex advertised capability at a specific speed

=item --autoneg (true|false)

Set the interface auto negotiation advertised capability

=item --pause (true|false)

Set the interface pause advertised capability

=back

=head1 EXAMPLES

=over

=item dabba interface get capabilities

Output the capabilities of all available network interfaces.

=item dabba interface modify capabilities --id eth0 --speed 100 --half-duplex false

Stop advertising half duplex capability at 100Mbps on 'eth0' (if supported)

=item dabba interface modify capabilities --id eth0 --pause true

Enable advertised pause capability on 'eth0' (if supported)

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
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <linux/ethtool.h>

#include <dabba/rpc.h>
#include <dabba/cli.h>
#include <dabba/help.h>
#include <dabba/macros.h>

/**
 * \internal
 * \brief Protobuf closure to print interface capabilities list in YAML
 * \param[in]           result	        Pointer to interface capabilities list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_capabilities_list_print(const
					      Dabba__InterfaceCapabilitiesList *
					      result, void *closure_data)
{
	Dabba__InterfaceCapabilities *capabilitiesp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		capabilitiesp = result->list[a];
		printf("    - name: %s\n", capabilitiesp->id->name);
		printf("    ");
		__rpc_error_code_print(capabilitiesp->status->code);
		printf("      capabilities:\n");
		printf("        port: {%s: %s, %s: %s, %s: %s, %s: %s, %s: %s",
		       port2str(PORT_TP),
		       print_tf(capabilitiesp->tp),
		       port2str(PORT_AUI),
		       print_tf(capabilitiesp->aui),
		       port2str(PORT_MII),
		       print_tf(capabilitiesp->mii),
		       port2str(PORT_FIBRE),
		       print_tf(capabilitiesp->fibre),
		       port2str(PORT_BNC), print_tf(capabilitiesp->bnc));
		printf("}\n");
		printf("        supported:\n");
		printf("          autoneg: %s\n",
		       print_tf(capabilitiesp->supported_opt->autoneg));
		printf("          pause: %s\n",
		       print_tf(capabilitiesp->supported_opt->pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(capabilitiesp->supported_speed->ethernet->half),
		       print_tf(capabilitiesp->supported_speed->ethernet->full),
		       print_tf(capabilitiesp->supported_speed->fast_ethernet->
				half),
		       print_tf(capabilitiesp->supported_speed->fast_ethernet->
				full),
		       print_tf(capabilitiesp->supported_speed->gbps_ethernet->
				half),
		       print_tf(capabilitiesp->supported_speed->gbps_ethernet->
				full),
		       print_tf(capabilitiesp->supported_speed->
				_10gbps_ethernet->full));
		printf("        advertised:\n");
		printf("          autoneg: %s\n",
		       print_tf(capabilitiesp->advertising_opt->autoneg));
		printf("          pause: %s\n",
		       print_tf(capabilitiesp->advertising_opt->pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(capabilitiesp->advertising_speed->ethernet->
				half),
		       print_tf(capabilitiesp->advertising_speed->ethernet->
				full),
		       print_tf(capabilitiesp->advertising_speed->
				fast_ethernet->half),
		       print_tf(capabilitiesp->advertising_speed->
				fast_ethernet->full),
		       print_tf(capabilitiesp->advertising_speed->
				gbps_ethernet->half),
		       print_tf(capabilitiesp->advertising_speed->
				gbps_ethernet->full),
		       print_tf(capabilitiesp->advertising_speed->
				_10gbps_ethernet->full));
		printf("        link-partner advertised:\n");
		printf("          autoneg: %s\n",
		       print_tf(capabilitiesp->lp_advertising_opt->autoneg));
		printf("          pause: %s\n",
		       print_tf(capabilitiesp->lp_advertising_opt->pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(capabilitiesp->lp_advertising_speed->ethernet->
				half),
		       print_tf(capabilitiesp->lp_advertising_speed->ethernet->
				full),
		       print_tf(capabilitiesp->lp_advertising_speed->
				fast_ethernet->half),
		       print_tf(capabilitiesp->lp_advertising_speed->
				fast_ethernet->full),
		       print_tf(capabilitiesp->lp_advertising_speed->
				gbps_ethernet->half),
		       print_tf(capabilitiesp->lp_advertising_speed->
				gbps_ethernet->full),
		       print_tf(capabilitiesp->lp_advertising_speed->
				_10gbps_ethernet->full));
	}

	*status = 1;
}

/**
 * \brief Invoke interface capabilities get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the capabilities of all available interfaces
 */

int rpc_interface_capabilities_get(ProtobufCService * service,
				   const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_capabilities_get(service, id_list,
							 interface_capabilities_list_print,
							 &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \internal
 * \brief Invoke interface capabilities list RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the capabilities of all available interfaces
 */

static int rpc_interface_capabilities_modify(ProtobufCService * service,
					     const Dabba__InterfaceCapabilities
					     * capabilitiesp)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(capabilitiesp);

	dabba__dabba_service__interface_capabilities_modify(service,
							    capabilitiesp,
							    rpc_error_code_print,
							    &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare interface capabilities modify RPC from \c argv
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return Returns 0 on success, else otherwise.
 */

int cmd_interface_capabilities_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_PORT_AUI,
		OPT_PORT_BNC,
		OPT_PORT_FIBRE,
		OPT_PORT_MII,
		OPT_PORT_TP,
		OPT_PORT_SPEED,
		OPT_PORT_HALF_DUPLEX,
		OPT_PORT_FULL_DUPLEX,
		OPT_PORT_AUTONEG,
		OPT_PORT_PAUSE,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{port2str(PORT_AUI), required_argument, NULL, OPT_PORT_AUI},
		{port2str(PORT_BNC), required_argument, NULL, OPT_PORT_BNC},
		{port2str(PORT_FIBRE), required_argument, NULL, OPT_PORT_FIBRE},
		{port2str(PORT_MII), required_argument, NULL, OPT_PORT_MII},
		{port2str(PORT_TP), required_argument, NULL, OPT_PORT_TP},
		{"speed", required_argument, NULL, OPT_PORT_SPEED},
		{"half-duplex", required_argument, NULL, OPT_PORT_HALF_DUPLEX},
		{"full-duplex", required_argument, NULL, OPT_PORT_FULL_DUPLEX},
		{"autoneg", required_argument, NULL, OPT_PORT_AUTONEG},
		{"pause", required_argument, NULL, OPT_PORT_PAUSE},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	uint32_t speed = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceCapabilities capabilities =
	    DABBA__INTERFACE_CAPABILITIES__INIT;

	Dabba__InterfaceSpeedCapabilites speed_cap =
	    DABBA__INTERFACE_SPEED_CAPABILITES__INIT;
	Dabba__InterfaceOptionCapabilites opt_cap =
	    DABBA__INTERFACE_OPTION_CAPABILITES__INIT;
	Dabba__InterfaceDuplexCapabilites duplex_cap =
	    DABBA__INTERFACE_DUPLEX_CAPABILITES__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_PORT_AUI:
			rc = str2bool(optarg, &capabilities.aui);

			if (rc)
				goto out;

			capabilities.has_aui = 1;
			break;
		case OPT_PORT_BNC:
			rc = str2bool(optarg, &capabilities.bnc);

			if (rc)
				goto out;

			capabilities.has_bnc = 1;
			break;
		case OPT_PORT_FIBRE:
			rc = str2bool(optarg, &capabilities.fibre);

			if (rc)
				goto out;

			capabilities.has_fibre = 1;
			break;
		case OPT_PORT_MII:
			rc = str2bool(optarg, &capabilities.mii);

			if (rc)
				goto out;

			capabilities.has_mii = 1;
			break;
		case OPT_PORT_TP:
			rc = str2bool(optarg, &capabilities.tp);

			if (rc)
				goto out;

			capabilities.has_tp = 1;
			break;
		case OPT_PORT_SPEED:
			rc = str2speed(optarg, &speed);

			if (rc)
				goto out;
			break;
		case OPT_PORT_FULL_DUPLEX:
			rc = str2bool(optarg, &duplex_cap.full);

			if (rc)
				goto out;

			duplex_cap.has_full = 1;
			break;
		case OPT_PORT_HALF_DUPLEX:
			rc = str2bool(optarg, &duplex_cap.half);

			if (rc)
				goto out;

			duplex_cap.has_half = 1;
			break;
		case OPT_PORT_AUTONEG:
			rc = str2bool(optarg, &opt_cap.autoneg);

			if (rc)
				goto out;

			opt_cap.has_autoneg = 1;
			break;
		case OPT_PORT_PAUSE:
			rc = str2bool(optarg, &opt_cap.pause);

			if (rc)
				goto out;

			opt_cap.has_pause = 1;
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
			capabilities.id = malloc(sizeof(*capabilities.id));

			if (!capabilities.id)
				return ENOMEM;

			dabba__interface_id__init(capabilities.id);
			capabilities.id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	switch (speed) {
	case SPEED_10:
		speed_cap.ethernet = &duplex_cap;
		break;
	case SPEED_100:
		speed_cap.fast_ethernet = &duplex_cap;
		break;
	case SPEED_1000:
		speed_cap.gbps_ethernet = &duplex_cap;
		break;
	case SPEED_10000:
		speed_cap._10gbps_ethernet = &duplex_cap;
		break;
	default:
		rc = EINVAL;
		goto out;
		break;
	}

	capabilities.status = &err;
	capabilities.advertising_speed = &speed_cap;
	capabilities.advertising_opt = &opt_cap;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_capabilities_modify(service, &capabilities);
	else
		rc = EINVAL;
 out:
	free(capabilities.id);
	return rc;
}
