// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_context.c - test for clip_new_context
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
#include <linux/vserver/context.h>

#define ARGS_TYPE xid_t xid, vs_caps bcap, vs_caps ccap, vs_flags cflags
#define ARGS xid, bcap, ccap, cflags
#define TEST_FUN clip_new_context

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting xid = %u, bcap = 0x%llx, ccap = 0x%llx, "
			"cflags = 0x%llx\nResults:\n",
			xid, bcap, ccap, cflags);
}

static char *status_matches[] = { "Cap", NULL };
static char *vinfo_matches[] = { "XID", "BCaps", "CCaps", "CFlags", NULL };

static int call_test(ARGS_TYPE);


static int
show_results(void)
{
	if (_read_file("/proc/self/status", status_matches))
		return -1;
	if (_read_file("/proc/self/vinfo", vinfo_matches))
		return -1;
	return 0;
}

int 
main(void)
{
	if (call_test(500, 0xffffffff, 0, 0))
		return EXIT_FAILURE;
	if (call_test(500, (1<<CAP_DAC_OVERRIDE), 0, 0))
		return EXIT_FAILURE;
	if (call_test(500, (1<<CAP_SETUID), VXC_SYSLOG, VXF_VIRT_MEM))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
