/**
 * \file sock-filter.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libdabba-rpc/rpc.h>

/**
 * \brief Free and clear a protobuf socket filter
 * \param[in] pbuf_sfp	protobuf socket filter to free and clear
 */
/* TODO add this function to libdabba-rpc */
void sock_filter_destroy(Dabba__SockFprog * const pbuf_sfp)
{
	size_t a;

	assert(pbuf_sfp);

	if (pbuf_sfp->n_filter)
		assert(pbuf_sfp->filter);

	for (a = 0; a < pbuf_sfp->n_filter; a++)
		free(pbuf_sfp->filter[a]);

	free(pbuf_sfp->filter);
	dabba__sock_fprog__init(pbuf_sfp);
}

/**
 * \brief Parse a socket filter file to a protobuf socket filter
 * \param[in]  sf_path	socket filter file path
 * \param[out] pbuf_sfp	resulting protobuf socket filter
 * \return \c errno on failure, 0 on success.
 */

int sock_filter_parse(const char *const sf_path,
		      Dabba__SockFprog * const pbuf_sfp)
{
	int ret, rc = 0;
	char buff[128] = { 0 };
	Dabba__SockFilter *sf;
	Dabba__SockFilter **tmp;

	assert(pbuf_sfp);
	assert(sf_path);

	FILE *fp = fopen(sf_path, "r");

	if (!fp)
		return errno;

	memset(buff, 0, sizeof(buff));

	while (fgets(buff, sizeof(buff), fp)) {
		/* We're using evil sscanf, so we have to assure
		   that we don't get into a buffer overflow ... */
		buff[sizeof(buff) - 1] = 0;

		/* Not a socket filter instruction. Skip this line */
		if (buff[0] != '{')
			continue;

		sf = malloc(sizeof(*sf));

		if (!sf) {
			rc = ENOMEM;
			break;
		}

		dabba__sock_filter__init(sf);

		ret = sscanf(buff, "{ 0x%x, %d, %d, 0x%08x },",
			     (unsigned int *)((void *)&(sf->code)),
			     (int *)((void *)&(sf->jt)),
			     (int *)((void *)&(sf->jf)), &(sf->k));

		/* No valid bpf opcode format or a syntax error */
		if (ret != 4) {
			rc = EINVAL;
			break;
		}

		tmp =
		    realloc(pbuf_sfp->filter,
			    sizeof(*pbuf_sfp->filter) * (pbuf_sfp->n_filter +
							 1));

		if (!tmp) {
			rc = ENOMEM;
			break;
		}

		pbuf_sfp->filter = tmp;
		pbuf_sfp->filter[pbuf_sfp->n_filter] = sf;
		pbuf_sfp->n_filter++;

		memset(buff, 0, sizeof(buff));
	}

	fclose(fp);

	return rc;
}
