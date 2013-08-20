
#ifndef INTERFACE_PAUSE_H
#define	INTERFACE_PAUSE_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_pause_get(Dabba__DabbaService_Service * service,
				const Dabba__InterfaceIdList * id_list,
				Dabba__InterfacePauseList_Closure
				closure, void *closure_data);

void dabbad_interface_pause_modify(Dabba__DabbaService_Service * service,
				   const Dabba__InterfacePause * pause,
				   Dabba__ErrorCode_Closure
				   closure, void *closure_data);

#endif				/* INTERFACE_PAUSE_H */
