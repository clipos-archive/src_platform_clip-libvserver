// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_jail.c - test for clip_jailself
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

#define ARGS_TYPE xid_t xid, vs_caps bcap, vs_flags cflags, const char *addr, const char *path
#define ARGS xid, bcap, cflags, addr, path
#define TEST_FUN clip_jailself

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting xid = %u, bcap = 0x%llx, cflags = 0x%llx, "
			"addr = %s, path = %s\nResults:\n",
			xid, bcap, cflags, addr, path);
}

static char *status_matches[] = { "Cap", NULL };
static char *vinfo_matches[] = { "XID", "BCaps", "CCaps", "CFlags", NULL };
static char *ninfo_matches[] = { "NID", "NFlags", "V4Root", NULL };

static int call_test(ARGS_TYPE);

#include <errno.h>
#include <dirent.h>

static int
_read_root(void)
{
	DIR *d;
	struct dirent *ent;
	int count = 0;

	d = opendir("/");
	if (!d) {
		perror("opendir");
		return -1;
	}
	fputs("\tChild root listing:", stdout);
	errno = 0;
	while ((ent = readdir(d))) {
		if (!(count++ % 4))
			fputs("\n\t    ", stdout);
		printf("%s ", ent->d_name);
	}
	closedir(d);
	putchar('\n');
	return (errno) ? -1 : 0;
}
		

static int
show_results(void)
{
	if (_read_file("/proc/self/status", status_matches)) {
		puts("\tCannot read proc files from child : "
				"no /proc in its tree?");
		goto root;
	}
	if (_read_file("/proc/self/vinfo", vinfo_matches))
		return -1;
	if (_read_file("/proc/self/ninfo", ninfo_matches))
		return -1;
root:
	_read_root();
	return 0;
}

int 
main(void)
{
	if (call_test(500, 0xffffffff, 0, "192.168.0.1", "/"))
		return EXIT_FAILURE;
	if (call_test(500, (1<<CAP_DAC_OVERRIDE), VXF_VIRT_MEM, "192.168.0.1", "/var/empty"))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

#include "test_common.h"
