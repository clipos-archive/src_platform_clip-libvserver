// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/*
 *  syscall-net.h - libclipvserver net syscalls header
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_SYSCALL_NET_H
#define _CLIP_SYSCALL_NET_H

#include "syscall.h"
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
/* Dirty hack to avoid including the whole kernel... */
#define _LINUX_IN_H
#define _LINUX_IN6_H
#include <vserver/network_cmd.h>
#include <vserver/switch.h>
#include <vserver/network.h>

static inline int
_clip_net_create(nid_t nid)
{
	struct vcmd_net_create data = { 
		.flagword = NXF_STATE_ADMIN |
				NXF_LBACK_REMAP |
				NXF_HIDE_LBACK |
				NXF_SINGLE_IP |
				NXF_HIDE_NETIF
	};

	return _clip_vserver(VCMD_net_create, nid, &data);
}

static inline int
_clip_net_migrate(nid_t nid)
{
	return _clip_vserver(VCMD_net_migrate, nid, NULL);
}

static inline int
_E_aton(const char *src, uint32_t *dest)
{
	struct in_addr tmp;
	if (!inet_aton(src, &tmp)) {
		fprintf(stderr, "inet_aton error on %s\n", src);
		return -1;
	}
	*dest = tmp.s_addr;
	return 0;
}

static inline int
_clip_net_set_nflags(nid_t nid, vs_flags nflags)
{
	struct vcmd_net_flags_v0 data = {
		.flagword = nflags & ~NXF_STATE_SETUP,
		.mask = nflags | NXF_STATE_SETUP,
	};
	return _clip_vserver(VCMD_set_nflags, nid, &data);
}

#endif /* _CLIP_SYSCALL_NET_H */
