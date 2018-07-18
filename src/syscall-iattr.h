// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/*
 *  syscall-iattr.h - libclipvserver iattr syscalls header
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_SYSCALL_IATTR_H
#define _CLIP_SYSCALL_IATTR_H

#include "syscall.h"
/* Argh */
#define __user
#include <vserver/inode_cmd.h>
#include <vserver/switch.h>

static inline int
_clip_set_iattr(const char *name, uint32_t set, uint32_t unset)
{
	struct vcmd_ctx_iattr_v1 data = {
		.name = name,
		.flags = set & ~unset,
		.mask = set | unset,
	};
	return _clip_vserver(VCMD_set_iattr, 0, &data);
}

#undef __user
#endif /* _CLIP_SYSCALL_IATTR_H */
