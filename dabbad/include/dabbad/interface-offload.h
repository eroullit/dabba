
#ifndef INTERFACE_OFFLOAD_H
#define	INTERFACE_OFFLOAD_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_offload_get(Dabba__DabbaService_Service * service,
				  const Dabba__InterfaceIdList * id_list,
				  Dabba__InterfaceOffloadList_Closure
				  closure, void *closure_data);

void dabbad_interface_offload_modify(Dabba__DabbaService_Service * service,
				     const Dabba__InterfaceOffload * offloadp,
				     Dabba__ErrorCode_Closure closure,
				     void *closure_data);

#endif				/* INTERFACE_OFFLOAD_H */
