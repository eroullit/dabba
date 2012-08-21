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

#ifndef THREAD_H
#define	THREAD_H

int sched_policy_value_get(const char *const policy_name);
const char *sched_policy_key_get(const int policy_value);
int sched_policy_default_get(void);
int sched_prio_default_get(void);
void sched_cpu_affinty_default_get(cpu_set_t * mask);
void str_to_cpu_affinity(char *str, cpu_set_t * mask);
int cmd_thread(int argc, const char **argv);

#endif				/* THREAD_H */
