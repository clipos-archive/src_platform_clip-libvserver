// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-net.c  
 *  Network context management functions for libclipvserver
 *  @author Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  Copyright (C) 2007 SGDN/DCSSI
 *  @n
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "clip-vserver.h"
#include "syscall-net.h"

inline int
clip_net_add_addrs(nid_t nid, const char **addrs, const char **masks)
{
	unsigned int i = 0;
	struct vcmd_net_addr_ipv4_v1 data; 

	while (addrs[i] && masks[i]) {
		memset(&data, 0, sizeof(data));
		data.type = NXA_TYPE_ADDR;
		data.flags = 0;
			
		if (_E_aton(addrs[i], (uint32_t*)&data.ip)) 
			return -1;
		if (_E_aton(masks[i], (uint32_t*)&data.mask)) 
			return -1;

		if (_clip_vserver(VCMD_net_add_ipv4_v1, nid, &data) != 0)
			return -1;

		++i;
	}
	
	return 0;
}
	
inline int
clip_net_set_bcast(nid_t nid, const char *addr) 
{
	struct vcmd_net_addr_ipv4_v1 data;
	memset(&data, 0, sizeof(data));
	data.type = NXA_TYPE_ADDR | NXA_MOD_BCAST;
	
	if (_E_aton(addr, (uint32_t*)&data.ip))
		return -1;
	
	if (_clip_vserver(VCMD_net_add_ipv4_v1, nid, &data) != 0)
		return -1;
	return 0;
}

inline int
clip_net_set_lback(nid_t nid, const char *addr) 
{
	struct vcmd_net_addr_ipv4_v1 data;
	memset(&data, 0, sizeof(data));
	data.type = NXA_TYPE_ADDR | NXA_MOD_LBACK;
	
	if (_E_aton(addr, (uint32_t*)&data.ip))
		return -1;
	
	if (_clip_vserver(VCMD_net_add_ipv4_v1, nid, &data) != 0)
		return -1;
	return 0;
}

int 
clip_net_del_addrs(nid_t nid)
{
	struct vcmd_net_addr_ipv4_v1 data;
	memset(&data, 0, sizeof(data));
	data.type = NXA_TYPE_ANY;
	
	return _clip_vserver(VCMD_net_rem_ipv4_v1, nid, &data);
}

int
clip_new_net_context_multi(nid_t nid, const char **addrs, const char **masks,
					vs_flags nflags)
{
	if (_clip_net_create(nid) == -1) {
		perror("net_create");
		return -1;
	}
	if (clip_net_add_addrs(nid, addrs, masks) == -1) {
		perror("net_add");
		return -1;
	}
	if (clip_net_set_bcast(nid, addrs[0]) == -1) {
		perror("net_set_bcast");
		return -1;
	}
	if (clip_net_set_lback(nid, addrs[0]) == -1) {
		perror("net_set_lback");
		return -1;
	}
	if (_clip_net_set_nflags(nid, nflags) == -1) {
		perror("net_set_nflags");
		return -1;
	}

	return 0;
}

int
clip_new_net_context(nid_t nid, const char *addr, const char *mask, vs_flags nflags)
{
	const char *addrs[2] = { addr, NULL };
	const char *masks[2] = { mask, NULL };

	return clip_new_net_context_multi(nid, addrs, masks, nflags);
}
