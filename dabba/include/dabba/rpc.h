
#ifndef DABBA_RPC_H
#define	DABBA_RPC_H

#include <libdabba-rpc/rpc.h>

ProtobufCService *dabba_rpc_client_connect(const char *const name,
					   const ProtobufC_RPC_AddressType
					   type);
void dabba_rpc_call_is_done(protobuf_c_boolean * is_done);
void __rpc_error_code_print(const int error_code);
void rpc_header_print(const char *const title);
void rpc_error_code_print(const Dabba__ErrorCode * const result,
			  void *closure_data);

#endif				/* DABBA_RPC_H */
