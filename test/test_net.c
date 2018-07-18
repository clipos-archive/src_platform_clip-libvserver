// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_net.c - test for clip_new_net_context
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

#define ARGS_TYPE nid_t nid, const char *addr, const char *mask, vs_flags nflags
#define ARGS nid, addr, mask, nflags
#define TEST_FUN clip_new_net_context

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting nid = %u, addr = %s, mask = %s, nflags = 0x%llx\n "
			"Results:\n", nid, addr, mask, nflags);
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

	if (call_test(500, "192.168.0.1", "255.255.255.0", 0))
		return EXIT_FAILURE;
	if (call_test(500, "192.168.0.1", "255.255.255.128", 0))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
