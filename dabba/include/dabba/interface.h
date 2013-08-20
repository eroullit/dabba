
#ifndef INTERFACE_H
#define	INTERFACE_H

#include <libdabba-rpc/rpc.h>

int rpc_interface_get(int argc, const char **argv,
		      int (*const rpc) (ProtobufCService * service,
					const Dabba__InterfaceIdList *
					id_list));
int cmd_interface(int argc, const char **argv);

#endif				/* INTERFACE_H */
