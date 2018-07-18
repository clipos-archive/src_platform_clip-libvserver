// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-iattr.c  
 *  Inode attributes management functions for libclipvserver
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
#include "clip-vserver.h"
#include "syscall-iattr.h"

int
clip_set_iattr(const char *name, uint32_t set, uint32_t unset)
{
	if (set & unset) {
		fputs("clip_set_iattr: intersection of set and unset"
			" is not empty", stderr);
		return -1;
	}
	if (_clip_set_iattr(name, set, unset) == -1) {
		perror("_clip_set_iattr");
		return -1;
	}
	return 0;
}
