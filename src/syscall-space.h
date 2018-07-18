// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/*
 *  syscall-space.h - libclipvserver namespace syscalls header
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_SYSCALL_SPACE_H
#define _CLIP_SYSCALL_SPACE_H

#include "syscall.h"
#include <vserver/space_cmd.h>
#include <vserver/switch.h>
#include <linux/sched.h>

#define BASE_NS_MASK CLONE_NEWNS|CLONE_NEWUTS|CLONE_NEWIPC

static inline int
_clip_enter_space(xid_t xid, uint64_t mask)
{
	/* Default flags : all namespaces */
	struct vcmd_space_mask_v1 data = { 
		.mask = mask,
	};
	return _clip_vserver(VCMD_enter_space_v1, xid, &data);
}

static inline int
_clip_set_space(xid_t xid, uint64_t mask)
{
	/* Default flags : all namespaces */
	struct vcmd_space_mask_v1 data = { 
		.mask = mask,
	};	
	return _clip_vserver(VCMD_set_space_v1, xid, &data);
}

#endif /* _CLIP_SYSCALL_SPACE_H */
