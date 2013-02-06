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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <linux/ethtool.h>

#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/macros.h>

extern const char *port2str(const uint8_t port);

static void display_interface_capabilities_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_capabilities_list_print(const
					      Dabba__InterfaceCapabilitiesList *
					      result, void *closure_data)
{
	Dabba__InterfaceCapabilities *capabilitiesp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_capabilities_header();

	for (a = 0; result && a < result->n_list; a++) {
		capabilitiesp = result->list[a];
		printf("    - name: %s\n", capabilitiesp->id->name);
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
		       print_tf(capabilitiesp->supported_speed->
				fast_ethernet->half),
		       print_tf(capabilitiesp->supported_speed->
				fast_ethernet->full),
		       print_tf(capabilitiesp->supported_speed->
				gbps_ethernet->half),
		       print_tf(capabilitiesp->supported_speed->
				gbps_ethernet->full),
		       print_tf(capabilitiesp->
				supported_speed->_10gbps_ethernet->full));
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
		       print_tf(capabilitiesp->advertising_speed->
				ethernet->half),
		       print_tf(capabilitiesp->advertising_speed->
				ethernet->full),
		       print_tf(capabilitiesp->
				advertising_speed->fast_ethernet->half),
		       print_tf(capabilitiesp->
				advertising_speed->fast_ethernet->full),
		       print_tf(capabilitiesp->
				advertising_speed->gbps_ethernet->half),
		       print_tf(capabilitiesp->
				advertising_speed->gbps_ethernet->full),
		       print_tf(capabilitiesp->
				advertising_speed->_10gbps_ethernet->full));
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
		       print_tf(capabilitiesp->lp_advertising_speed->
				ethernet->half),
		       print_tf(capabilitiesp->lp_advertising_speed->
				ethernet->full),
		       print_tf(capabilitiesp->
				lp_advertising_speed->fast_ethernet->half),
		       print_tf(capabilitiesp->
				lp_advertising_speed->fast_ethernet->full),
		       print_tf(capabilitiesp->
				lp_advertising_speed->gbps_ethernet->half),
		       print_tf(capabilitiesp->
				lp_advertising_speed->gbps_ethernet->full),
		       print_tf(capabilitiesp->
				lp_advertising_speed->_10gbps_ethernet->full));
	}

	*status = 1;
}

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
