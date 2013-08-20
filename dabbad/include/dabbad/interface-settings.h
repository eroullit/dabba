
#ifndef INTERFACE_SETTINGS_H
#define	INTERFACE_SETTINGS_H

#include <libdabba-rpc/rpc.h>

void dabbad_interface_settings_get(Dabba__DabbaService_Service * service,
				   const Dabba__InterfaceIdList * id_list,
				   Dabba__InterfaceSettingsList_Closure
				   closure, void *closure_data);

void dabbad_interface_settings_modify(Dabba__DabbaService_Service * service,
				      const Dabba__InterfaceSettings *
				      settingsp,
				      Dabba__ErrorCode_Closure closure,
				      void *closure_data);

#endif				/* INTERFACE_SETTINGS_H */
