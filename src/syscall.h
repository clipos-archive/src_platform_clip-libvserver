// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/*
 *  syscall.h - libclipvserver vserver syscall header
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_SYSCALL_H
#define _CLIP_SYSCALL_H

#include <unistd.h>
#include <sys/syscall.h>
#include "clip-vserver.h"

static inline int
_clip_vserver(uint32_t cmd, uint32_t id, void *data)
{
	return syscall(__NR_vserver, cmd, id, data);
}

#endif /* _CLIP_SYSCALL_H */
