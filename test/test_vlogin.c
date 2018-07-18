// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_vlogin.c - test for clip_vlogin
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
#define TEST_FUN clip_vlogin

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting vlogin\nResults:\n");
}

static int call_test(ARGS_TYPE);


static inline int
_show_stdio(void)
{
	char buf[256];
	int len;

	len = readlink("/proc/self/fd/0", buf, 256);
	if (len == -1) {
		perror("readlink 0");
		return -1;
	}
	printf("\tfd 0 : %.*s\n", len, buf);
	len = readlink("/proc/self/fd/1", buf, 256);
	if (len == -1) {
		perror("readlink 1");
		return -1;
	}
	printf("\tfd 1 : %.*s\n", len, buf);
	len = readlink("/proc/self/fd/2", buf, 256);
	if (len == -1) {
		perror("readlink 2");
		return -1;
	}
	printf("\tfd 2 : %.*s\n", len, buf);
	
	return 0;
}

static int
show_results(void)
{
	char buf[8];
	char *ptr;

	puts("Child fds:");
	if (_show_stdio())
		return -1;
	puts("\tPlease type a few chars...");
	read(STDIN_FILENO, buf, 8);
	if ((ptr = strchr(buf, '\n')))
		*ptr = '\0';
	
	printf("\tChild read \"%.8s\"\n",buf);
	fflush(stdout);
	return 0;
}

static int
_show_father(void)
{
	puts("\nFather fds:");
	return _show_stdio();
}
#define parent_hook() (void)_show_father()

int 
main(void)
{
	if (call_test())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"

