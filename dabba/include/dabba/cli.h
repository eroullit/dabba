/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#ifndef CLI_H
#define	CLI_H

int str2bool(const char *const str, int *const val);
int str2speed(const char *const str, uint32_t * const speed);
int str2duplex(const char *const str, int *const duplex);

char *duplex2str(const int duplex);

int str2sched_policy(const char *const policy_name);
const char *sched_policy2str(const int policy);
const char *thread_type2str(const int type);
const char *port2str(const uint8_t port);

#endif				/* CLI_H */
