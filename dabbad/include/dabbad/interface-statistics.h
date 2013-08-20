
#ifndef INTERFACE_STATISTICS_H
#define	INTERFACE_STATISTICS_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_statistics_get(Dabba__DabbaService_Service * service,
				     const Dabba__InterfaceIdList * id_list,
				     Dabba__InterfaceStatisticsList_Closure
				     closure, void *closure_data);

#endif				/* INTERFACE_STATISTICS_H */
