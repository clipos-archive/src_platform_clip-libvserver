// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-namespace.c 
 *  Namespace management functions for libclipvserver
 *  @author Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  Copyright (C) 2007 SGDN/DCSSI
 *  @n
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>

#include "clip-vserver.h"
#include "syscall-space.h"

/**
 * @internal
 * @brief Common part of @a clip_new_namespace() and 
 * @a clip_new_namespace_opts().
 * @param opts Flags to pass to SYS_clone.
 * @return 0 on success, -1 on failure.
 */
static int
do_new_namespace(int opts, clip_ns_callback_t cb, void *data)
{
	pid_t pid, ret;
	int status;

	signal(SIGCHLD, SIG_DFL);

	pid = syscall(SYS_clone, opts, NULL);

	switch (pid) {
		case -1:
			perror("clone");
			return -1;
			break;
		case 0:
			return 0;
		default:
			if (cb && (*cb)(pid, data))
				exit(EXIT_FAILURE);
			ret = waitpid(pid, &status, 0);
			if (ret == -1) {
				perror("wait");
				exit(EXIT_FAILURE);
			}
			if (WIFEXITED(status))
				exit(WEXITSTATUS(status));

			if (WIFSIGNALED(status)) {
				kill(getpid(), WTERMSIG(status));
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
			
			break;
	}
	return -1;
}

int
clip_new_namespace(void)
{
	return do_new_namespace(BASE_NS_MASK|SIGCHLD, NULL, NULL);
}

static inline int
new_namespace_opts(int opts, clip_ns_callback_t cb, void *data)
{
	int ns = BASE_NS_MASK|SIGCHLD;
	
	if (opts & CLIP_VSERVER_PIDNS)
		ns |= CLONE_NEWPID;
	if (opts & CLIP_VSERVER_USRNS)
		ns |= CLONE_NEWUSER;
#ifdef NET_NS
	if (opts & CLIP_VSERVER_NETNS)
		ns |= CLONE_NEWNET;
#else
	if (opts & CLIP_VSERVER_NETNS) {
		fputs("net namespaces are not supported\n", stderr);
		return -1;
	}
#endif

	return do_new_namespace(ns, cb, data);
}

int
clip_new_namespace_opts(int opts)
{
	return new_namespace_opts(opts, NULL, NULL);
}

int 
clip_new_namespace_callback(int opts, clip_ns_callback_t cb, void *data)
{
	return new_namespace_opts(opts, cb, data);
}
