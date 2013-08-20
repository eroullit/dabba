
#ifndef INTERFACE_STATUS_H
#define	INTERFACE_STATUS_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_status_get(Dabba__DabbaService_Service * service,
				 const Dabba__InterfaceIdList * id_list,
				 Dabba__InterfaceStatusList_Closure
				 closure, void *closure_data);

void dabbad_interface_status_modify(Dabba__DabbaService_Service * service,
				    const Dabba__InterfaceStatus * statusp,
				    Dabba__ErrorCode_Closure closure,
				    void *closure_data);

#endif				/* INTERFACE_STATUS_H */
