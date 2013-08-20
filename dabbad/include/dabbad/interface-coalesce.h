
#ifndef INTERFACE_COALESCE_H
#define	INTERFACE_COALESCE_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_coalesce_get(Dabba__DabbaService_Service * service,
				   const Dabba__InterfaceIdList * id_list,
				   Dabba__InterfaceCoalesceList_Closure
				   closure, void *closure_data);

void dabbad_interface_coalesce_modify(Dabba__DabbaService_Service * service,
				      const Dabba__InterfaceCoalesce *
				      coalescep,
				      Dabba__ErrorCode_Closure closure,
				      void *closure_data);

#endif				/* INTERFACE_COALESCE_H */
