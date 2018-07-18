// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_context_enter.c - test for clip_enter_context
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
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <sched.h>

#define ARGS_TYPE xid_t xid, vs_caps bcap, vs_caps ccap, vs_flags cflags
#define ARGS xid, bcap, ccap, cflags
#define TEST_FUN spawn_context_enter

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting xid = %u\nResults:\n", xid);
}

static char *status_matches[] = { "Cap", NULL };
static char *vinfo_matches[] = { "XID", "BCaps", "CCaps", "CFlags", NULL };
static char *ninfo_matches[] = { "NID", NULL };

static int call_test(ARGS_TYPE);

static pid_t child_pid;

static void
sighandle(int sig)
{
	printf("\t\t%d got signal\n", getpid());
	exit((sig == SIGTERM) ? 0 : -1);
}

static int
spawn_context_enter(ARGS_TYPE)
{
	pid_t pid;
	char c;
	int fds[2];

	if (pipe(fds) == -1) {
		perror("pipe");
		return -1;
	}
	pid = fork();
	switch (pid) {
		case -1: 
			perror("fork");
			return -1;
		case 0: 
			(void)close(fds[0]);
			if (signal(SIGTERM, sighandle) == SIG_ERR) {
				perror("signal");
				exit(-1);
			}
			if (clip_new_net_context(xid, "192.168.0.1", 
							"255.255.255.0", 0)) {
				fputs("Network context creation failed\n", 
								stderr);
				exit(-1);
			}
			if (clip_new_context(ARGS)) {
				fputs("Context creation failed\n", stderr);
				exit(-1);
			}
			if (write(fds[1], "a", 1) != 1) {
				perror("write");
				exit(-1);
			}

			usleep(3000000);
			exit(-1);
		default:
			child_pid = pid;
			(void)close(fds[1]);
			if (read(fds[0], &c, 1) < 0) {
				perror("read");
				if(kill(child_pid, SIGTERM))
					perror("kill");
				return -1;
			}
			if (clip_enter_context(xid)) {
				fputs("Context enter failed\n", stderr);
				if(kill(child_pid, SIGTERM))
					perror("kill");;
				return -1;
			}
			return 0;
	}
}
			

static int
show_results(void)
{
	DIR *d;
	struct dirent *ent;

	puts("\tParent status:");
	if (_read_file("/proc/self/status", status_matches))
		goto error;
	if (_read_file("/proc/self/vinfo", vinfo_matches))
		goto error;
	if (_read_file("/proc/self/ninfo", ninfo_matches))
		goto error;
	
	printf("\tParent pid %d, child pid %d\n", getpid(), child_pid);
	fputs("\t/proc seen from parent:", stdout);
	d = opendir("/proc");
	if (!d) {
		perror("opendir");
		goto error;
	}
	errno = 0;
	while ((ent = readdir(d))) {
		if (*ent->d_name < '1' || *ent->d_name > '9')
			continue;
		fputs("\n\t    ", stdout);
		printf("%s ", ent->d_name);
	}
	closedir(d);
	putchar('\n');
	if(kill(child_pid, SIGTERM))
		perror("kill");
	sched_yield();
	return (errno) ? -1 : 0;

error:
	if(kill(child_pid, SIGTERM))
		perror("kill");
	return -1;
}

int 
main(void)
{
	if (call_test(502, 0xffffffff, 0, 0))
		return EXIT_FAILURE;
	if (call_test(503, (1<<CAP_SETUID), VXC_SYSLOG, VXF_VIRT_MEM))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
