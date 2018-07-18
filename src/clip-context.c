// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-context.c Context management functions for libclipvserver
 *  @author Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Copyright (C) 2010-2014 SGDSN/ANSSI
 *  @n
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>

#include "clip-vserver.h"
#include "syscall-ctx.h"
#include "syscall-space.h"
#include "syscall-net.h"

#ifndef MS_REC
/**
 * @internal
 * @brief Recusive mount option.
 */
#define MS_REC 0x4000
#endif 

/**
 * @internal
 * @brief Create a context and set its namespace.
 *
 * Leaves the new context in the STATE_SETUP|STATE_INIT state.
 */
static inline int
new_context(xid_t xid, uint64_t nsopts)
{
	if (_clip_ctx_create(xid) == -1) {
		perror("ctx_create");
		return -1;
	}

	if (_clip_set_space(xid, nsopts) == -1) {
		perror("set_space");
		return -1;
	}
	return 0;
}

/**
 * @internal
 * @brief Setup POSIX capabilities, vserver capabilities and flags for a security context.
 *
 * The target context must have the STATE_SETUP flag. On success, the STATE_SETUP and 
 * STATE_INIT flags are removed from the context.
 */
static inline int
do_setup(xid_t xid, vs_caps bcaps, vs_caps ccaps, vs_flags cflags)
{
	if (_clip_ctx_set_ccaps(xid, ccaps) == -1) {
		perror("set_ccaps");
		return -1;
	}	
	if (_clip_ctx_set_bcaps(xid, bcaps) == -1) {
		perror("set_bcaps");
		return -1;
	}
	if (_clip_ctx_set_cflags(xid, cflags) == -1) {
		perror("set_cflags");
		return -1;
	}

	return 0;
}


int
clip_new_context(xid_t xid, vs_caps bcaps, vs_caps ccaps, vs_flags flags)
{
	int ret;

	ret = new_context(xid, BASE_NS_MASK|CLONE_FS);
	if (ret)
		return ret;

	return do_setup(xid, bcaps, ccaps, flags);
}

int
clip_new_context_nsopts(xid_t xid, vs_caps bcaps, vs_caps ccaps, 
					vs_flags flags, int nsopts)
{
	int ret;

	uint64_t ns = BASE_NS_MASK|CLONE_FS;
	
	if (nsopts & CLIP_VSERVER_PIDNS)
		ns |= CLONE_NEWPID;
	if (nsopts & CLIP_VSERVER_USRNS)
		ns |= CLONE_NEWUSER;
#ifdef NET_NS
	if (nsopts & CLIP_VSERVER_NETNS)
		ns |= CLONE_NEWNET;
#else
	if (nsopts & CLIP_VSERVER_NETNS) {
		fputs("net namespaces are not supported\n", stderr);
		return -1;
	}
#endif

	ret = new_context(xid, ns);
	if (ret)
		return ret;

	return do_setup(xid, bcaps, ccaps, flags);
}

int
clip_enter_context_nons(xid_t xid) 
{
	if (_clip_net_migrate(xid) == -1) {
		perror("net_migrate");
		return -1;
	}

	if (_clip_ctx_migrate(xid) == -1) {
		perror("ctx_migrate");
		return -1;
	}

	return 0;
}

int
clip_enter_security_context(xid_t xid) 
{
	if (_clip_ctx_migrate(xid) == -1) {
		perror("ctx_migrate");
		return -1;
	}

	return 0;
}

int
clip_enter_context(xid_t xid) 
{
	if (_clip_enter_space(xid, 0) == -1) {
		perror("enter_space");
		return -1;
	}

	return clip_enter_context_nons(xid);
}

int 
clip_enter_namespace(xid_t xid)
{
	if (_clip_enter_space(xid, 0) == -1) {
		perror("enter_space");
		return -1;
	}
	return 0;
}

int
clip_enter_some_namespaces(xid_t xid, uint64_t mask)
{
	if (_clip_enter_space(xid, mask) == -1) {
		perror("enter_space");
		return -1;
	}
	return 0;
}

int
clip_jailself(xid_t xid, vs_caps bcaps, vs_flags flags, 
		const char *addr, const char *path)
{
	int ret;
	ret = clip_new_net_context(xid, addr, "255.255.255.0", 0ULL);
	if (ret)
		return ret;

	ret = clip_new_namespace();
	if (ret)
		return ret;

	ret = mount(path, "/", "rootfs", MS_BIND|MS_REC|MS_RDONLY|MS_NODEV, NULL);
	if (ret) {
		perror("recursive root mount");
		return -1;
	}

	ret = chdir(path);
	if (ret) {
		perror("chdir");
		return -1;
	}
	ret = chroot(".");
	if (ret) {
		perror("chroot");
		return -1;
	}


	return clip_new_context(xid, bcaps, 0ULL, flags);
	
}
