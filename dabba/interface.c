/**
 * \file interface.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2012
 * \date 2012
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 *
 */

/* __LICENSE_HEADER_END__ */

/*

=head1 NAME

dabba-interface - Manage network interface

=head1 SYNOPSIS

dabba interface <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the available network interfaces.

=head1 COMMANDS

=over

=item list

Fetch and print information about currenty supported interfaces.
The output is formatted in YAML.

=item modify

Modify status of available network interfaces

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --up (true|false)

Activate or shutdown a specific interface.

=item --running (true|false)

Mark the interface as operational.

=item --promiscuous (true|false)

Enable or disable the 'promiscuous' mode of an interface.
If selected, all packets on the network will be received by the interface.

=back

=head1 EXAMPLES

=over

=item dabba interface list

Output the list of network interface which can be used by dabba.

=item dabba interface modify --id eth0 --up true --running true --promiscuous true

Modify the status of the interface 'eth0' to be up and running and alse
listen to all incoming packets.

=back

=head1 AUTHOR

Written by Emmanuel Roullit <emmanuel.roullit@gmail.com>

=head1 BUGS

=over

=item Please report bugs to <https://github.com/eroullit/dabba/issues>

=item dabba project project page: <https://github.com/eroullit/dabba>

=back

=head1 COPYRIGHT

=over

=item Copyright © 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

=item This is free software: you are free to change and redistribute it.

=item There is NO WARRANTY, to the extent permitted by law.

=back

=cut

*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <libdabba/macros.h>
#include <libdabba/strlcpy.h>
#include <dabba/dabba.h>
#include <dabba/ipc.h>
#include <dabba/help.h>

#ifndef print_tf
#define print_tf(expr) (expr) ? "true" : "false"
#endif

enum interface_modify_option {
	OPT_INTERFACE_UP,
	OPT_INTERFACE_RUNNING,
	OPT_INTERFACE_PROMISC,
	OPT_INTERFACE_ID,
};

static const char *ethtool_port_str_get(const uint8_t port)
{
	static const char *const port_str[] = {
		[PORT_TP] = "tp",
		[PORT_AUI] = "aui",
		[PORT_MII] = "mii",
		[PORT_FIBRE] = "fibre",
		[PORT_BNC] = "bnc",
		[PORT_DA] = "da"
	};

	return port < sizeof(port_str) ? port_str[port] : "unknown";
}

static struct option *interface_modify_options_get(void)
{
	static struct option interface_modify_option[] = {
		{"up", required_argument, NULL, OPT_INTERFACE_UP},
		{"running", required_argument, NULL, OPT_INTERFACE_RUNNING},
		{"promiscuous", required_argument, NULL, OPT_INTERFACE_PROMISC},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{NULL, 0, NULL, 0},
	};

	return interface_modify_option;
}

static void display_interface_list_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

/**
 * \brief Print fetch interface information in a YAML format
 * \param[in]           interface_msg	interface information IPC message
 * \param[in]           elem_nr		number of interfaces to report
 */

static void display_interface_list(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_list *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_LIST_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_LIST);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_list[a];
		printf("    - name: %s\n", iface->name);
		printf("      status: {");
		printf("link: %s, ", print_tf(iface->link));
		printf("up: %s, ", print_tf(iface->up == TRUE));
		printf("running: %s, ", print_tf(iface->running == TRUE));
		printf("promiscuous: %s, ", print_tf(iface->promisc == TRUE));
		printf("loopback: %s", print_tf(iface->loopback == TRUE));
		printf("}\n");
		printf("      statistics:\n");
		printf("          rx: {");
		printf("byte: %u, ", iface->rx.byte);
		printf("packet: %u, ", iface->rx.packet);
		printf("error: %u, ", iface->rx.error);
		printf("dropped: %u, ", iface->rx.dropped);
		printf("compressed: %u", iface->rx.compressed);
		printf("}\n");
		printf("          tx: {");
		printf("byte: %u, ", iface->tx.byte);
		printf("packet: %u, ", iface->tx.packet);
		printf("error: %u, ", iface->tx.error);
		printf("dropped: %u, ", iface->tx.dropped);
		printf("compressed: %u", iface->tx.compressed);
		printf("}\n");
		printf("          rx error: {");
		printf("fifo: %u, ", iface->rx_error.fifo);
		printf("frame: %u, ", iface->rx_error.frame);
		printf("crc: %u, ", iface->rx_error.crc);
		printf("length: %u, ", iface->rx_error.length);
		printf("missed: %u, ", iface->rx_error.missed);
		printf("overflow: %u", iface->rx_error.over);
		printf("}\n");
		printf("          tx error: {");
		printf("fifo: %u, ", iface->tx_error.fifo);
		printf("carrier: %u, ", iface->tx_error.carrier);
		printf("heartbeat: %u, ", iface->tx_error.heartbeat);
		printf("window: %u, ", iface->tx_error.window);
		printf("aborted: %u", iface->tx_error.aborted);
		printf("}\n");
	}
}

static void display_interface_driver(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_driver *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_DRIVER_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_DRIVER);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_driver[a];
		printf("    - name: %s\n", iface->name);
		printf("        driver name: %s\n", iface->driver_info.driver);
		printf("        driver version: %s\n",
		       iface->driver_info.version);
		printf("        firmware version: %s\n",
		       iface->driver_info.fw_version);
		printf("        bus info: %s\n", iface->driver_info.bus_info);
	}
}

static void display_interface_settings(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_settings *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_SETTINGS_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_SETTINGS);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_settings[a];
		printf("    - name: %s\n", iface->name);
		printf("      settings:\n");
		printf("        speed: %u\n",
		       ethtool_cmd_speed(&iface->settings));
		printf("        duplex: %s\n",
		       iface->settings.duplex == DUPLEX_FULL ? "full" : "half");
		printf("        autoneg: %s\n",
		       print_tf(iface->settings.autoneg == AUTONEG_ENABLE));
		printf("        port: %s\n",
		       ethtool_port_str_get(iface->settings.port));
		printf("        max rx packet: %u\n", iface->settings.maxrxpkt);
		printf("        max tx packet: %u\n", iface->settings.maxtxpkt);
	}
}

static void display_interface_capabilities(const struct dabba_ipc_msg *const
					   msg)
{
	size_t a;
	const struct dabba_interface_settings *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_SETTINGS_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_SETTINGS);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_settings[a];
		printf("    - name: %s\n", iface->name);
		printf("      capabilities:\n");
		printf("        port: {%s: %s, %s: %s, %s: %s, %s: %s, %s: %s",
		       ethtool_port_str_get(PORT_TP),
		       print_tf(iface->settings.supported & SUPPORTED_TP),
		       ethtool_port_str_get(PORT_AUI),
		       print_tf(iface->settings.supported & SUPPORTED_AUI),
		       ethtool_port_str_get(PORT_MII),
		       print_tf(iface->settings.supported & SUPPORTED_MII),
		       ethtool_port_str_get(PORT_FIBRE),
		       print_tf(iface->settings.supported & SUPPORTED_FIBRE),
		       ethtool_port_str_get(PORT_BNC),
		       print_tf(iface->settings.supported & SUPPORTED_BNC));
		printf("}\n");
		printf("        supported:\n");
		printf("          autoneg: %s\n",
		       print_tf(iface->settings.supported & SUPPORTED_Autoneg));
		printf("          pause: %s\n",
		       print_tf(iface->settings.supported & SUPPORTED_Pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(iface->
				settings.supported & SUPPORTED_10baseT_Half),
		       print_tf(iface->
				settings.supported & SUPPORTED_10baseT_Full),
		       print_tf(iface->
				settings.supported & SUPPORTED_100baseT_Half),
		       print_tf(iface->
				settings.supported & SUPPORTED_100baseT_Full),
		       print_tf(iface->
				settings.supported & SUPPORTED_1000baseT_Half),
		       print_tf(iface->
				settings.supported & SUPPORTED_1000baseT_Full),
		       print_tf(iface->
				settings.supported &
				SUPPORTED_10000baseT_Full));
		printf("        advertised:\n");
		printf("          autoneg: %s\n",
		       print_tf(iface->
				settings.advertising & ADVERTISED_Autoneg));
		printf("          pause: %s\n",
		       print_tf(iface->
				settings.advertising & ADVERTISED_Pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(iface->
				settings.advertising & ADVERTISED_10baseT_Half),
		       print_tf(iface->
				settings.advertising & ADVERTISED_10baseT_Full),
		       print_tf(iface->
				settings.advertising &
				ADVERTISED_100baseT_Half),
		       print_tf(iface->
				settings.advertising &
				ADVERTISED_100baseT_Full),
		       print_tf(iface->
				settings.advertising &
				ADVERTISED_1000baseT_Half),
		       print_tf(iface->
				settings.advertising &
				ADVERTISED_1000baseT_Full),
		       print_tf(iface->
				settings.advertising &
				ADVERTISED_10000baseT_Full));
		printf("        link-partner advertised:\n");
		printf("          autoneg: %s\n",
		       print_tf(iface->
				settings.lp_advertising & ADVERTISED_Autoneg));
		printf("          pause: %s\n",
		       print_tf(iface->
				settings.lp_advertising & ADVERTISED_Pause));
		printf("          speed:\n");
		printf("            10:    {half: %s, full: %s}\n"
		       "            100:   {half: %s, full: %s}\n"
		       "            1000:  {half: %s, full: %s}\n"
		       "            10000: {half: false, full: %s}\n",
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_10baseT_Half),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_10baseT_Full),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_100baseT_Half),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_100baseT_Full),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_1000baseT_Half),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_1000baseT_Full),
		       print_tf(iface->settings.lp_advertising &
				ADVERTISED_10000baseT_Full));
	}
}

static void display_interface_pause(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_pause *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_PAUSE_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_PAUSE);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_pause[a];
		printf("    - name: %s\n", iface->name);
		printf("      pause:\n");
		printf("        autoneg: %s\n", print_tf(iface->pause.autoneg));
		printf("        rx: %s\n", print_tf(iface->pause.rx_pause));
		printf("        tx: %s\n", print_tf(iface->pause.tx_pause));
	}
}

static void display_interface_coalesce(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_coalesce *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_COALESCE_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_COALESCE);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_coalesce[a];
		printf("    - name: %s\n", iface->name);
		printf("      coalesce:\n");
		printf("        packet rate high: %u\n",
		       iface->coalesce.pkt_rate_high);
		printf("        packet rate low: %u\n",
		       iface->coalesce.pkt_rate_low);
		printf("        rate sample interval: %u\n",
		       iface->coalesce.rate_sample_interval);
		printf("        stats block: %u\n",
		       iface->coalesce.stats_block_coalesce_usecs);
		printf("        rx:\n");
		printf("            adaptive: %s\n",
		       print_tf(iface->coalesce.use_adaptive_rx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", iface->coalesce.rx_coalesce_usecs);
		printf("irq: %u, ", iface->coalesce.rx_coalesce_usecs_irq);
		printf("high: %u, ", iface->coalesce.rx_coalesce_usecs_high);
		printf("low: %u", iface->coalesce.rx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", iface->coalesce.rx_max_coalesced_frames);
		printf("irq: %u, ",
		       iface->coalesce.rx_max_coalesced_frames_irq);
		printf("high: %u, ",
		       iface->coalesce.rx_max_coalesced_frames_high);
		printf("low: %u", iface->coalesce.rx_max_coalesced_frames_low);
		printf("}\n");
		printf("        tx:\n");
		printf("            adaptive: %s\n",
		       print_tf(iface->coalesce.use_adaptive_tx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", iface->coalesce.tx_coalesce_usecs);
		printf("irq: %u, ", iface->coalesce.tx_coalesce_usecs_irq);
		printf("high: %u, ", iface->coalesce.tx_coalesce_usecs_high);
		printf("low: %u", iface->coalesce.tx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", iface->coalesce.tx_max_coalesced_frames);
		printf("irq: %u, ",
		       iface->coalesce.tx_max_coalesced_frames_irq);
		printf("high: %u, ",
		       iface->coalesce.tx_max_coalesced_frames_high);
		printf("low: %u", iface->coalesce.tx_max_coalesced_frames_low);
		printf("}\n");
	}
}

static void display_interface_offload(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_offload *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_OFFLOAD_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_OFFLOAD);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_offload[a];
		printf("    - name: %s\n", iface->name);
		printf("      offload:\n");
		printf("        rx checksum: %s\n", print_tf(iface->rx_csum));
		printf("        tx checksum: %s\n", print_tf(iface->tx_csum));
		printf("        scatter gather: %s\n", print_tf(iface->sg));
		printf("        tcp segment: %s\n", print_tf(iface->tso));
		printf("        udp fragment: %s\n", print_tf(iface->ufo));
		printf("        generic segmentation: %s\n",
		       print_tf(iface->gso));
		printf("        generic receive: %s\n", print_tf(iface->gro));
		printf("        rx hashing: %s\n", print_tf(iface->rxhash));
	}
}

/**
 * \brief Get current interface list information and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 *
 * This function prepares an IPC message to query the supported network
 * interfaces present on the system. Once the message is sent, it waits for the
 * dabba daemon to reply.
 */

int cmd_interface_list(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_LIST;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_list);
}

/**
 * \brief Get interface driver information and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_driver(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_DRIVER;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_driver);
}

/**
 * \brief Get interface hardware settings and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_settings(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_SETTINGS;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_settings);
}

/**
 * \brief Get interface hardware capabilities and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_capabilities(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_SETTINGS;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_capabilities);
}

/**
 * \brief Get interface pause parameters and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_pause(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_PAUSE;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_pause);
}

/**
 * \brief Get interface coalesce parameters and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_coalesce(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_COALESCE;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_coalesce);
}

/**
 * \brief Get interface offload parameters and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_offload(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_OFFLOAD;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_offload);
}

static int prepare_interface_modify_query(int argc, char **argv, struct dabba_interface_list
					  *ifconf_msg)
{
	int ret, rc = 0;

	assert(ifconf_msg);

	ifconf_msg->up = UNSET;
	ifconf_msg->running = UNSET;
	ifconf_msg->promisc = UNSET;
	ifconf_msg->loopback = UNSET;

	while ((ret =
		getopt_long_only(argc, argv, "", interface_modify_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_INTERFACE_UP:
			ifconf_msg->up = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_RUNNING:
			ifconf_msg->running = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_PROMISC:
			ifconf_msg->promisc = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_ID:
			strlcpy(ifconf_msg->name, optarg,
				sizeof(ifconf_msg->name));
			break;
		default:
			show_usage(interface_modify_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Modify parameters of a supported interface
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_modify(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_INTERFACE_MODIFY;
	msg.msg_body.elem_nr = 1;

	rc = prepare_interface_modify_query(argc, (char **)argv,
					    msg.msg_body.msg.interface_list);

	if (rc)
		return rc;

	return dabba_ipc_msg(&msg);
}

/**
 * \brief Parse which interface sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the interface sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_interface(int argc, const char **argv)
{
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct interface_commands[] = {
		{"list", cmd_interface_list},
		{"driver", cmd_interface_driver},
		{"settings", cmd_interface_settings},
		{"capabilities", cmd_interface_capabilities},
		{"pause", cmd_interface_pause},
		{"coalesce", cmd_interface_coalesce},
		{"offload", cmd_interface_offload},
		{"modify", cmd_interface_modify}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(interface_commands); i++) {
		if (!strcmp(interface_commands[i].cmd, cmd))
			return run_builtin(&interface_commands[i], argc, argv);
	}

	return ENOSYS;
}
