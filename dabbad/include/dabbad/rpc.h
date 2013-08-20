
#ifndef DABBAD_RPC_H
#define	DABBAD_RPC_H

#include <libdabba-rpc/rpc.h>

ProtobufC_RPC_Server *dabbad_rpc_server_start(const char *const name,
					      const ProtobufC_RPC_AddressType
					      type);
void dabbad_rpc_server_stop(ProtobufC_RPC_Server * server);
int dabbad_rpc_msg_poll(void);

#endif				/* DABBAD_RPC_H */
