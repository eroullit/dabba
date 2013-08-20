
#ifndef SOCK_FILTER_H
#define	SOCK_FILTER_H

void sock_filter_destroy(Dabba__SockFprog * pbuf_sfp);
int sock_filter_parse(const char *const sf_path,
		      Dabba__SockFprog * const pbuf_sfp);

#endif				/* SOCK_FILTER_H */
