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

#ifndef INTERFACE_CAPABILITIES_H
#define	INTERFACE_CAPABILITIES_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_capabilities_get(Dabba__DabbaService_Service * service,
				       const Dabba__InterfaceIdList * id_list,
				       Dabba__InterfaceCapabilitiesList_Closure
				       closure, void *closure_data);

void dabbad_interface_capabilities_modify(Dabba__DabbaService_Service * service,
					  const Dabba__InterfaceCapabilities *
					  capabilitiesp,
					  Dabba__ErrorCode_Closure closure,
					  void *closure_data);

#endif				/* INTERFACE_CAPABILITIES_H */
