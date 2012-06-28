/**
 * \file misc.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <libdabba/strlcpy.h>

/**
 * \brief Path to current process open file descriptor information
 */

#define PROC_FD_PATH "/proc/self/fd/"

/**
 * \brief Get file path from an open file descriptor
 * \param[in]           fd	        File descriptor
 * \param[out]          path		Pointer to the path string buffer
 * \param[in]           path_len	Path string buffer size in bytes
 * \return 0 on success, else on failure.
 *
 * This function retrieves the absolute path of a file from its file descriptor.
 * It mainly checks the device/inode numbers of the file descriptor and the file
 * to correlate them.
 */

int fd_to_path(const int fd, char *path, const size_t path_len)
{
	char proc_path[PATH_MAX], fdstr[16];
	struct stat fd_stat, path_stat;
	DIR *dir;
	struct dirent *dir_entry;
	char *resolved_path;
	int rc = 0;

	assert(fd > 0);
	assert(path);
	assert(path_len);

	memset(proc_path, 0, sizeof(proc_path));
        memset(fdstr, 0, sizeof(fdstr));
	memset(&fd_stat, 0, sizeof(fd_stat));
	memset(&resolved_path, 0, sizeof(resolved_path));
	memset(&path_stat, 0, sizeof(path_stat));

	if (snprintf(fdstr, sizeof(fdstr), "%i", fd) <= 0)
		return errno;

	dir = opendir(PROC_FD_PATH);

	if (!dir)
		return errno;

	/* /proc/self/fd/<fdstr> is a symlink to the actual file */
	for (dir_entry = readdir(dir); dir_entry; dir_entry = readdir(dir)) {
		if (strcmp(dir_entry->d_name, fdstr) == 0)
			break;
	}

	if (!dir_entry) {
		rc = ENOENT;
		goto out;
	}

	if (snprintf
	    (proc_path, sizeof(proc_path), "%s%s", PROC_FD_PATH, dir_entry->d_name) <= 0) {
		rc = errno;
		goto out;
	}

	if (fstat(fd, &fd_stat) < 0 || stat(proc_path, &path_stat) < 0) {
		rc = errno;
		goto out;
	}

	/* filenames are not accurate, interface and inode ID need to checked too */
	if (fd_stat.st_dev != path_stat.st_dev
	    || fd_stat.st_ino != path_stat.st_ino) {
		rc = EINVAL;
		goto out;
	}

	/* resolve /proc/self/fd/<fdstr> symlink now */
	resolved_path = realpath(proc_path, NULL);

	if (!resolved_path) {
		rc = errno;
		goto out;
	}

	strlcpy(path, resolved_path, path_len);

	free(resolved_path);

 out:
	closedir(dir);
	return rc;
}
