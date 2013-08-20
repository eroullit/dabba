/**
 * \file sock-filter.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef SOCK_FILTER_H
#define	SOCK_FILTER_H

#include <linux/filter.h>

int ldab_sock_filter_attach(const int sock, const struct sock_fprog *const sfp);
int ldab_sock_filter_detach(const int sock);
int ldab_sock_filter_is_valid(const struct sock_fprog *const bpf);

#endif				/* SOCK_FILTER_H */
