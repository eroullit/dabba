
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
