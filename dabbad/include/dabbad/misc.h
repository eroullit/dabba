
#ifndef MISC_H
#define	MISC_H

#include <paths.h>

int fd_to_path(const int fd, char *path, const size_t path_len);
int create_pidfile(const char *const pidfile);
int core_enable(void);

#endif				/* MISC_H */
