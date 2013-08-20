/**
 * \file sock-filter.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <linux/filter.h>

#include <libdabba/sock-filter.h>
#include <libdabba-rpc/rpc.h>

/**
 * \brief Free and clear a native socket filter
 * \param[in] sfp	socket filter to free and clear
 */

void dabbad_sfp_destroy(struct sock_fprog *const sfp)
{
	assert(sfp);
	free(sfp->filter);
	sfp->len = 0;
}

/**
 * \brief Free and clear a protobuf socket filter
 * \param[in] pbuf_sfp	protobuf socket filter to free and clear
 */

void dabbad_pbuf_sfp_destroy(Dabba__SockFprog * pbuf_sfp)
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
 * \brief Convert a protobuf socket filter to a native socket filter
 * \param[in]  pbuf_sf	protobuf socket filter to convert
 * \param[out] sfp	result native socket filter
 * \return \c ENOMEM if the system is out-of-memory
 *         \c EINVAL if the produced socket filter is invalid
 *         0 on success.
 * \note if the function is successful, \c sfp must be freed with
 *       \c dabbad_sfp_destroy() afterwards.
 * \see dabbad_sfp_destroy
 */

int dabbad_pbuf_sfp_2_sfp(const Dabba__SockFprog * const pbuf_sf,
			  struct sock_fprog *const sfp)
{
	size_t a;

	assert(pbuf_sf);
	assert(sfp);

	sfp->filter = calloc(pbuf_sf->n_filter, sizeof(*sfp->filter));

	if (!sfp->filter)
		return ENOMEM;

	for (a = 0; a < pbuf_sf->n_filter; a++) {
		sfp->filter[a].code = pbuf_sf->filter[a]->code;
		sfp->filter[a].jt = pbuf_sf->filter[a]->jt;
		sfp->filter[a].jf = pbuf_sf->filter[a]->jf;
		sfp->filter[a].k = pbuf_sf->filter[a]->k;
	}

	sfp->len = pbuf_sf->n_filter;

	if (!ldab_sock_filter_is_valid(sfp)) {
		dabbad_sfp_destroy(sfp);
		return EINVAL;
	}

	return 0;
}

/**
 * \brief Convert a native socket filter to a protobuf socket filter
 * \param[in]  sfp	native socket filter to convert
 * \param[out] pbuf_sfp	resulting protobuf socket filter
 * \return \c ENOMEM if the system is out-of-memory, 0 on success.
 * \note if the function is successful, \c pbuf_sfp must be freed with
 *       \c dabbad_pbuf_sfp_destroy() afterwards.
 * \see dabbad_pbuf_sfp_destroy
 */

int dabbad_sfp_2_pbuf_sfp(const struct sock_fprog *const sfp,
			  Dabba__SockFprog * const pbuf_sfp)
{
	Dabba__SockFilter *sf;

	assert(sfp);
	assert(pbuf_sfp);

	pbuf_sfp->filter = calloc(sfp->len, sizeof(*pbuf_sfp->filter));

	if (!pbuf_sfp->filter)
		return ENOMEM;

	for (pbuf_sfp->n_filter = 0; pbuf_sfp->n_filter < sfp->len;
	     pbuf_sfp->n_filter++) {
		sf = malloc(sizeof(*sf));

		if (!sf) {
			dabbad_pbuf_sfp_destroy(pbuf_sfp);
			return ENOMEM;
		}

		dabba__sock_filter__init(sf);

		sf->code = sfp->filter[pbuf_sfp->n_filter].code;
		sf->jt = sfp->filter[pbuf_sfp->n_filter].jt;
		sf->jf = sfp->filter[pbuf_sfp->n_filter].jf;
		sf->k = sfp->filter[pbuf_sfp->n_filter].k;

		pbuf_sfp->filter[pbuf_sfp->n_filter] = sf;
	}

	return 0;
}
