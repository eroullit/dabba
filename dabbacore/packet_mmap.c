/**
 * \file packet_mmap.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2009-2011
 * \date 2011
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>

#include <dabbacore/macros.h>
#include <dabbacore/packet_mmap.h>
#include <dabbacore/nic.h>

/**
 * \internal
 * \brief Register a packet mmap to the kernel
 * \param[in] pkt_mmap	packet mmap to register
 * \return 0 on success, error code of setsockopt(2) on failure
 */

static int packet_mmap_register(const struct packet_mmap *const pkt_mmap)
{
	assert(pkt_mmap);

	if (setsockopt
	    (pkt_mmap->pf_sock, SOL_PACKET, pkt_mmap->type,
	     (void *)(&pkt_mmap->layout), sizeof(pkt_mmap->layout)) < 0) {
		return (errno);
	}

	return (0);
}

/**
 * \internal
 * \brief Unregister a packet mmap from the kernel
 * \param[in] pkt_mmap	packet mmap to unregister
 * \return 0 on success, error code of setsockopt(2) on failure
 */

static void packet_mmap_unregister(struct packet_mmap *pkt_mmap)
{
	assert(pkt_mmap);

	memset(&pkt_mmap->layout, 0, sizeof(pkt_mmap->layout));

	setsockopt(pkt_mmap->pf_sock, SOL_PACKET, pkt_mmap->type,
		   (void *)(&pkt_mmap->layout), sizeof(pkt_mmap->layout));
}

/**
 * \internal
 * \brief Map a packet mmap memory from the kernel to the userspace
 * \param[in,out] pkt_mmap	packet mmap to map to userspace
 * \return 0 on success, EINVAL on failure
 */

static int packet_mmap_mmap(struct packet_mmap *pkt_mmap)
{
	assert(pkt_mmap);

	pkt_mmap->buf =
	    mmap(0,
		 pkt_mmap->layout.tp_block_size *
		 pkt_mmap->layout.tp_block_nr, PROT_READ | PROT_WRITE,
		 MAP_SHARED | MAP_LOCKED, pkt_mmap->pf_sock, 0);

	if (pkt_mmap->buf == MAP_FAILED)
		return (EINVAL);

	return (0);
}

/**
 * \internal
 * \brief Unmap a packet mmap memory from the kernel to the userspace
 * \param[in,out] pkt_mmap	packet mmap to unmap to userspace
 */

static void packet_mmap_munmap(struct packet_mmap *pkt_mmap)
{
	assert(pkt_mmap);

	if (pkt_mmap->buf) {
		munmap(pkt_mmap->buf,
		       pkt_mmap->layout.tp_block_size *
		       pkt_mmap->layout.tp_block_nr);

		pkt_mmap->buf = NULL;
	}
}

/**
 * \internal
 * \brief Bind a packet mmap socket to the interface
 * \param[in] pkt_mmap	packet mmap to bind to the interface
 * \return 0 on success, error code from bind(2) on failure
 */

static int packet_mmap_bind(const struct packet_mmap *const pkt_mmap)
{
	struct sockaddr_ll sll;

	assert(pkt_mmap);

	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = pkt_mmap->ifindex;

	if (bind(pkt_mmap->pf_sock, (struct sockaddr *)&sll, sizeof(sll)) < 0)
		return (errno);

	/* Check error and if dev is ready */

	return (0);
}

/**
 * \internal
 * \brief Allocate and initialize a packet mmap I/O vector buffer
 * \param[in,out] pkt_mmap	packet mmap to create
 * \return 0 on success, ENOMEM on failure
 */

static int packet_mmap_vector_create(struct packet_mmap *pkt_mmap)
{
	size_t a;

	assert(pkt_mmap);
	assert(pkt_mmap->buf);

	pkt_mmap->vec =
	    calloc(pkt_mmap->layout.tp_frame_nr, sizeof(*pkt_mmap->vec));

	if (!pkt_mmap->vec)
		return (ENOMEM);

	for (a = 0; a < pkt_mmap->layout.tp_frame_nr; a++) {
		pkt_mmap->vec[a].iov_base =
		    &pkt_mmap->buf[a * pkt_mmap->layout.tp_frame_size];

		pkt_mmap->vec[a].iov_len = pkt_mmap->layout.tp_frame_size;
	}

	return (0);
}

/**
 * \internal
 * \brief Free packet mmap I/O vector buffer
 * \param[in,out] pkt_mmap	packet mmap to free
 */

static void packet_mmap_vector_destroy(struct packet_mmap *pkt_mmap)
{
	assert(pkt_mmap);

	free(pkt_mmap->vec);
}

/**
 * \brief Destroy a packet mmap
 * \param[in,out] pkt_mmap	packet mmap to destroy
 */

void packet_mmap_destroy(struct packet_mmap *pkt_mmap)
{
	assert(pkt_mmap);

	packet_mmap_vector_destroy(pkt_mmap);
	packet_mmap_munmap(pkt_mmap);
	packet_mmap_unregister(pkt_mmap);

	memset(pkt_mmap, 0, sizeof(*pkt_mmap));
}

/**
 * \brief Create a packet mmap
 * \param pkt_mmap[in,out]	packet mmap to create
 * \param dev[in]		Device name
 * \param pf_sock[in]		Open PF_PACKET socket
 * \param type[in]		Packet mmap type to create
 * \param frame_size[in]	Maximun packet mmap frame size
 * \param page_order[in]	Page order to use to create a block
 * \param size[in]		Total size of the packet mmap
 * \return 0 on success, else on failure
 *
 * To create a packet mmap, the input frame_size and the size must be a power of
 * two. Also the frame and block number must be bigger than zero.
 */

int packet_mmap_create(struct packet_mmap *pkt_mmap,
		       const char *const dev, const int pf_sock,
		       const enum packet_mmap_type type,
		       const enum packet_mmap_frame_size frame_size,
		       const uint8_t page_order, const size_t size)
{
	int rc = 0;

	assert(pkt_mmap);
	assert(dev);

	if (!is_power_of_2(frame_size) || !is_power_of_2(size))
		return EINVAL;

	memset(pkt_mmap, 0, sizeof(*pkt_mmap));

	rc = devname_to_ifindex(dev, &pkt_mmap->ifindex);

	if (rc != 0)
		goto out;

	pkt_mmap->type = type;
	pkt_mmap->pf_sock = pf_sock;

	pkt_mmap->layout.tp_frame_size = frame_size;
	pkt_mmap->layout.tp_block_size = getpagesize() << page_order;
	pkt_mmap->layout.tp_block_nr = (size / pkt_mmap->layout.tp_block_size);
	pkt_mmap->layout.tp_frame_nr =
	    pkt_mmap->layout.tp_block_size / pkt_mmap->layout.tp_frame_size *
	    pkt_mmap->layout.tp_block_nr;

	if (pkt_mmap->layout.tp_frame_nr == 0
	    || pkt_mmap->layout.tp_block_nr == 0) {
		rc = EINVAL;
		goto out;
	}

	rc = packet_mmap_register(pkt_mmap);

	if (rc != 0)
		goto out;

	rc = packet_mmap_mmap(pkt_mmap);

	if (rc != 0)
		goto out;

	rc = packet_mmap_vector_create(pkt_mmap);

	if (rc != 0)
		goto out;

	rc = packet_mmap_bind(pkt_mmap);

	if (rc != 0)
		goto out;

	return (0);
 out:
	packet_mmap_destroy(pkt_mmap);
	return (rc);
}
