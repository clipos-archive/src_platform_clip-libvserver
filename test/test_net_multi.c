// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_net_multi.c - test for clip_new_net_context_multi
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

#define ARGS_TYPE nid_t nid, const char **addr, const char **mask, vs_flags nflags
#define ARGS nid, addr, mask, nflags
#define TEST_FUN clip_new_net_context_multi

static inline void 
print_args(ARGS_TYPE)
{
	const char **ptr;
	printf("\nTesting nid = %u, addr = { ", nid);
	for (ptr = addr; *ptr; ptr++) {
		printf("%s, ", *ptr);
	}
	fputs(" }, mask = { ", stdout);
	for (ptr = mask; *ptr; ptr++) {
		printf("%s, ", *ptr);
	}
	printf(" } and nflags = 0x%llx\nResults:\n", nflags);
}

static char *ninfo_matches[] = { "NID", "NFlags", "V4Root", NULL };

static int call_test(ARGS_TYPE);


static int
show_results(void)
{
	if (_read_file("/proc/self/ninfo", ninfo_matches))
		return -1;
	return 0;
}

int 
main(void)
{
	char *addr1[] = { "192.168.0.1", NULL };
	char *mask1[] = { "255.255.255.0", NULL };
	char *addr2[] = { "192.168.0.1", "192.168.0.2", NULL };
	char *mask2[] = { "255.255.255.0", "255.255.0.0", NULL };
	char *addr3[] = { "192.168.0.1", NULL };
	char *mask3[] = { "255.255.254.0", NULL };

	if (call_test(500, (const char **)addr1, (const char **)mask1, 0))
		return EXIT_FAILURE;
	if (call_test(400, (const char **)addr2, (const char **)mask2, 0))
		return EXIT_FAILURE;
	if (call_test(300, (const char **)addr3, (const char **)mask3, 0))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
