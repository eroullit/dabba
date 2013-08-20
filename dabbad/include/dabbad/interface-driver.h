
#ifndef INTERFACE_DRIVER_H
#define	INTERFACE_DRIVER_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_driver_get(Dabba__DabbaService_Service * service,
				 const Dabba__InterfaceIdList * id_list,
				 Dabba__InterfaceDriverList_Closure
				 closure, void *closure_data);

#endif				/* INTERFACE_DRIVER_H */
