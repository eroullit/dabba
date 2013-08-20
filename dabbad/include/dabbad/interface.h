
#ifndef INTERFACE_H
#define	INTERFACE_H

struct nl_cache;
struct nl_sock;
struct rtnl_link;

struct nl_cache *link_cache_alloc(struct nl_sock **sock);
void link_cache_destroy(struct nl_sock *sock, struct nl_cache *cache);
void link_destroy(struct rtnl_link *link);

#endif				/* INTERFACE_H */
