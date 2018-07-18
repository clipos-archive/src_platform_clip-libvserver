// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/*
 *  syscall-ctx.h - libclipvserver context syscall- header
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_SYSCALL_CTX_H
#define _CLIP_SYSCALL_CTX_H

#include "syscall.h"
#include <vserver/context_cmd.h>
#include <vserver/switch.h>
#include <vserver/context.h>

static inline int
_clip_ctx_create(xid_t xid)
{
	struct vcmd_ctx_create data = { .flagword = VXF_INIT_SET };
	return _clip_vserver(VCMD_ctx_create, xid, &data);
}

static inline int
_clip_ctx_migrate(xid_t xid)
{
	/* NB : possible flags : VXM_SET_INIT, VXM_SET_REAPER */
	struct vcmd_ctx_migrate data = { .flagword = 0 }; 
	return _clip_vserver(VCMD_ctx_migrate, xid, &data);
}

static inline int
_clip_ctx_set_bcaps(xid_t xid, vs_caps bcaps)
{
	struct vcmd_bcaps data = {
		.bcaps = bcaps,
		.bmask = (vs_caps)-1,
	};
	return _clip_vserver(VCMD_set_bcaps, xid, &data);
}

static inline int
_clip_ctx_set_ccaps(xid_t xid, vs_caps ccaps)
{
	struct vcmd_ctx_caps_v1 data = {
		.ccaps = ccaps,
		.cmask = (vs_caps)-1,
	};
	return _clip_vserver(VCMD_set_ccaps, xid, &data);
}

static inline int
_clip_ctx_set_cflags(xid_t xid, vs_flags flags)
{
	struct vcmd_ctx_flags_v0 data = {
		.flagword = flags & ~VXF_STATE_INIT & ~VXF_STATE_SETUP,
		.mask = flags | VXF_STATE_INIT | VXF_STATE_SETUP,
	};
	return _clip_vserver(VCMD_set_cflags, xid, &data);
}
	
#endif /* _CLIP_SYSCALL_CTX_H */
