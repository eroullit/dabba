/**
 * \file sock-filter.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef DABBAD_SOCK_FILTER_H
#define	DABBAD_SOCK_FILTER_H

#include <libdabba-rpc/rpc.h>

struct sock_fprog;

void dabbad_sfp_destroy(struct sock_fprog *const sfp);
void dabbad_pbuf_sfp_destroy(Dabba__SockFprog * pbuf_sfp);
int dabbad_pbuf_sfp_2_sfp(const Dabba__SockFprog * const pbuf_sf,
			  struct sock_fprog *const sfp);
int dabbad_sfp_2_pbuf_sfp(const struct sock_fprog *const sfp,
			  Dabba__SockFprog * const pbuf_sfp);

#endif				/* DABBAD_SOCK_FILTER_H */
