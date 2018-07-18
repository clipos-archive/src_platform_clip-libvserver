// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_namespace.c - test for clip_new_namespace
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 **/

#include "test_prelude.h"

#include <linux/capability.h>
#include <clip-vserver.h>

#define ARGS_TYPE void
#define ARGS 
#define TEST_FUN clip_new_namespace

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting new_namespace\nResult:\n");
}

static char *mounts_matches[] = { "/", NULL };

static int call_test(ARGS_TYPE);

#include <sys/mount.h>

static int
show_results(void)
{
	puts("\tMounting /var/log on /var/empty in child.");
	if (mount("/var/log", "/var/empty", "none", MS_BIND, NULL)) {
		perror("mount");
		return -1;
	}
	puts("\t/proc/mounts in child:");
	if (_read_file("/proc/self/mounts", mounts_matches))
		return -1;
	return 0;
}

static int
show_father(void)
{
	puts("\n\t/proc/mounts in father:");
	return _read_file("/proc/self/mounts", mounts_matches);
}

#define parent_hook() show_father()

int 
main(void)
{
	if (call_test())
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

#include "test_common.h"
