/**
 * \file rpc.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include <libdabba-rpc/rpc.h>
#include <dabbad/interface.h>
#include <dabbad/interface-status.h>
#include <dabbad/interface-driver.h>
#include <dabbad/interface-pause.h>
#include <dabbad/interface-offload.h>
#include <dabbad/interface-settings.h>
#include <dabbad/interface-coalesce.h>
#include <dabbad/interface-capabilities.h>
#include <dabbad/interface-statistics.h>
#include <dabbad/thread.h>
#include <dabbad/capture.h>
#include <dabbad/replay.h>

/**
 * \brief Protobuf service structure used by dabbad
 */

static Dabba__DabbaService_Service dabba_service =
DABBA__DABBA_SERVICE__INIT(dabbad_);

/**
 * \brief Destroy a new dabbad RPC server instance
 * \param[in]       server	        Pointer to the server context
 */

void dabbad_rpc_server_stop(ProtobufC_RPC_Server * server)
{
	if (server)
		protobuf_c_rpc_server_destroy(server, 0);
}

/**
 * \brief Create a new dabbad RPC server instance
 * \param[in]       name	        String to RPC server address
 * \param[in]       type	        Tell if the RPC server listens to Unix or TCP sockets
 * \return Pointer to RPC server context on success, \c NULL on failure
 */

ProtobufC_RPC_Server *dabbad_rpc_server_start(const char *const name,
					      const ProtobufC_RPC_AddressType
					      type)
{
	int rc = 0;
	ProtobufC_RPC_Server *server;

	assert(name);
	assert(strlen(name));
	assert(type == PROTOBUF_C_RPC_ADDRESS_LOCAL
	       || type == PROTOBUF_C_RPC_ADDRESS_TCP);

	server = protobuf_c_rpc_server_new(type, name,
					   (ProtobufCService *) & dabba_service,
					   NULL);

	if (server && type == PROTOBUF_C_RPC_ADDRESS_LOCAL) {
		rc = chmod(name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

		if (rc) {
			dabbad_rpc_server_stop(server);
			server = NULL;
		}
	}

	return server;
}

/**
 * \brief Poll server for new RPC queries to process
 * \note This function polls for new RPC queries endlessly
 */

int dabbad_rpc_msg_poll(void)
{
	for (;;)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	return 0;
}
