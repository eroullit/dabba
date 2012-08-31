/**
 * \file dabbad.h
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

#ifndef DABBAD_H
#define	DABBAD_H

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sched.h>

/**
 * \brief Supported dabbad IPC message types
 */

enum dabba_tristate {
	FALSE,
	TRUE,
	UNSET
};

enum dabba_msg_type {
	DABBA_IFCONF,
	DABBA_IF_MODIFY,
	DABBA_CAPTURE_START,
	DABBA_CAPTURE_LIST,
	DABBA_CAPTURE_STOP,
	DABBA_THREAD_LIST,
	DABBA_THREAD_MODIFY,
	DABBA_THREAD_CAP_LIST
};

/**
 * \brief Dabbad raw message buffer
 */

struct dabba_msg_buf {
	uint8_t buf[1024];
};

/**
 * \brief Dabbad interface name buffer
 */

struct dabba_ifconf {
	char name[IFNAMSIZ];
	enum dabba_tristate up, running, promisc, loopback;
};

enum dabba_thread_flags {
	USE_CPU_MASK = 1 << 0,
	USE_SCHED_PRIO = 1 << 1,
	USE_SCHED_POLICY = 1 << 2
};

/**
 * \brief Dabbad thread message buffer
 */

struct dabba_thread {
	uint8_t usage_flags;
	cpu_set_t cpu;
	pthread_t id; /**< thread id */
	int16_t type; /**< thread type */
	int16_t sched_prio; /**< scheduling priority */
	int16_t sched_policy; /**< schenduling policy */
};

/**
 * \brief Dabbad thread scheduling capabilities message buffer
 */

struct dabba_thread_cap {
	int16_t policy;
	int16_t prio_min;
	int16_t prio_max;
};

/**
 * \brief Dabbad capture message buffer
 */

struct dabba_capture {
	char pcap_name[NAME_MAX]; /**< pcap file name */
	char dev_name[IFNAMSIZ]; /**< interface name */
	pthread_t id;
	uint64_t frame_nr; /**< number of frames to allocate */
	uint32_t frame_size; /**< maximum frame size to support */
};

#ifndef DABBA_IFCONF_MAX_SIZE
#define DABBA_IFCONF_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_ifconf))
#endif				/* DABBA_IFCONF_MAX_SIZE */

#ifndef DABBA_CAPTURE_MAX_SIZE
#define DABBA_CAPTURE_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_capture))
#endif				/* DABBA_CAPTURE_MAX_SIZE */

#ifndef DABBA_THREAD_MAX_SIZE
#define DABBA_THREAD_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_thread))
#endif				/* DABBA_THREAD_MAX_SIZE */

#ifndef DABBA_THREAD_CAP_MAX_SIZE
#define DABBA_THREAD_CAP_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_thread_cap))
#endif				/* DABBA_THREAD_CAP_MAX_SIZE */

/**
 * \brief Dabbad IPC message structure
 */

struct dabba_ipc_msg {
	long mtype; /**< IPC internal message type. Must be first. */

	struct dabba_msg {
		uint16_t type; /**< dabbad message type */
		uint16_t elem_nr; /**< dabbad #element present in the message */
		uint16_t offset; /**< dabbad element offset to start with */
		int16_t error; /**< error code when processing the message */

		union dabba_info {
			struct dabba_msg_buf buf;
			struct dabba_ifconf ifconf[DABBA_IFCONF_MAX_SIZE];
			struct dabba_capture capture[DABBA_CAPTURE_MAX_SIZE];
			struct dabba_thread thread[DABBA_THREAD_MAX_SIZE];
			struct dabba_thread_cap
			 thread_cap[DABBA_THREAD_CAP_MAX_SIZE];
		} msg; /**< message data */
	} msg_body;
};

/**
 * \brief Dabbad pid file path
 */

#ifndef DABBAD_PID_FILE
#define DABBAD_PID_FILE "/tmp/dabba.pid"
#endif				/* DABBAD_PID_FILE */

/**
 * \brief Dabbad IPC message queue id getter
 * \param[in] flags to use on msgget(2)
 * \return IPC message queue id or -1 on failure.
 */

static inline int dabba_get_ipc_queue_id(const int flags)
{
	key_t key = ftok(DABBAD_PID_FILE, 0xDABADABA);

	if (key == -1)
		return key;

	return msgget(key, flags);
}

static inline enum dabba_tristate dabba_tristate_parse(const char *const str)
{
	if (strcasecmp(str, "false") == 0)
		return FALSE;
	else if (strcasecmp(str, "true") == 0)
		return TRUE;
	else
		return UNSET;
}

#endif				/* DABBAD_H */
