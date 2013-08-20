/**
 * \file misc.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <dirent.h>

#include <unistd.h>

/**
 * \brief Path to current process open file descriptor information
 */

#ifndef PROC_FD_PATH
#define PROC_FD_PATH "/proc/self/fd/"
#endif				/* PROC_FD_PATH */

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
	    (proc_path, sizeof(proc_path), "%s%s", PROC_FD_PATH,
	     dir_entry->d_name) <= 0) {
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

	strncpy(path, resolved_path, path_len - 1);

	free(resolved_path);

 out:
	closedir(dir);
	return rc;
}

/**
 * \brief Create text where the current pid is written in
 * \param[in]           pidfile                PID file path
 * \return 0 on success, \c errno on failure.
 * \note if no file exists at the indicated path, a file is created.
 * \note if a file already exists at the indicated path, it is overwritten.
 */

int create_pidfile(const char *const pidfile)
{
	int pidfd, len, rc = 0;
	char pidstr[8];

	assert(pidfile);

	pidfd = open(pidfile, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP);

	if (pidfd < 0)
		return errno;

	len = snprintf(pidstr, sizeof(pidstr), "%u", getpid());

	if (write(pidfd, pidstr, len) != len)
		rc = errno;

	close(pidfd);

	return rc;
}

/**
 * \brief Fetch current core file size limit settings
 * \param[out]	lim 	Pointer to file size limit structure
 * \return 0 on success, \c errno on failure.
 */

static int core_rlimit_get(struct rlimit *const lim)
{
	assert(lim);
	return getrlimit(RLIMIT_CORE, lim) ? errno : 0;
}

/**
 * \brief Set core file size limit settings
 * \param[in]	lim 	Pointer to file size limit structure
 * \return 0 on success, \c errno on failure.
 */

static int core_rlimit_set(const struct rlimit *const lim)
{
	assert(lim);
	return setrlimit(RLIMIT_CORE, lim) ? errno : 0;
}

/**
 * \brief Enable dabba to generate a core dump file
 * \return 0 on success, \c errno on failure.
 * \note it raises the core file size limit to the maximum allowed
 * \note it sets process flags to produce core dumps
 */

int core_enable(void)
{
	struct rlimit lim;
	core_rlimit_get(&lim);
	lim.rlim_cur = lim.rlim_max;
	return (core_rlimit_set(&lim) || prctl(PR_SET_DUMPABLE, 1)) ? errno : 0;
}
