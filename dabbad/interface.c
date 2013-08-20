/**
 * \file interface.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


/* HACK prevent libnl3 include clash between <net/if.h> and <linux/if.h> */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif				/* _LINUX_IF_H */

#include <assert.h>
#include <errno.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <libdabba/macros.h>
#include <libdabba/interface.h>
#include <dabbad/interface.h>

/**
 * \brief Allocate the current interface cache from netlink
 * \param[in,out]       sock	        Double pointer to a netlink socket
 * \return Returns newly allocated netlink cache on success, NULL on failure.
 * \note use \c link_cache_destroy() to free allocated memory
 * \see link_cache_destroy()
 */

struct nl_cache *link_cache_alloc(struct nl_sock **sock)
{
	struct nl_cache *cache = NULL;

	assert(sock);

	*sock = nl_socket_alloc();

	if (!sock || nl_connect(*sock, NETLINK_ROUTE)
	    || rtnl_link_alloc_cache(*sock, AF_UNSPEC, &cache))
		return NULL;

	return cache;
}

/**
 * \brief Free interface cache returned by netlink
 * \param[in,out]       sock	        Double pointer to a netlink socket
 * \return Returns newly allocated netlink cache on success, NULL on failure.
 * \note use it to free memory allocated by \c link_cache_alloc()
 * \see link_cache_alloc()
 */

void link_cache_destroy(struct nl_sock *sock, struct nl_cache *cache)
{
	nl_cache_free(cache);
	nl_socket_free(sock);
}

void link_destroy(struct rtnl_link *link)
{
	if (link)
		nl_object_free(OBJ_CAST(link));
}
