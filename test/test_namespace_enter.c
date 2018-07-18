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
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <sched.h>

#define ARGS_TYPE xid_t xid
#define ARGS xid
#define TEST_FUN spawn_context_enter

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting xid = %u\nResults:\n", xid);
}

static char *mounts_matches[] = { "/", NULL };

static int call_test(ARGS_TYPE);

static pid_t child_pid;

static void
sighandle(int sig)
{
	printf("\t\t%d got signal\n", getpid());
	exit((sig == SIGTERM) ? 0 : -1);
}

#include <sys/mount.h>

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
			if (clip_new_namespace()) {
				fputs("Namespace creation failed\n", stderr);
				exit(-1);
			}
			puts("\tMounting /var/log on /var/empty in child.");
			if (mount("/var/log", "/var/empty", "none", 
							MS_BIND, NULL)) {
				perror("mount");
				return -1;
			}
			if (clip_new_context(ARGS, 0, 0, 0)) {
				fputs("Context creation failed\n", stderr);
				exit(-1);
			}

			puts("\t/proc/mounts in child:");
			if (_read_file("/proc/self/mounts", mounts_matches))
				exit(-1);

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
			if (clip_enter_namespace(xid)) {
				fputs("Namespace enter failed\n", stderr);
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

	
	printf("\tParent pid %d, child pid %d\n", getpid(), child_pid);
	puts("\n\t/proc/mounts in parent:");
	if(_read_file("/proc/self/mounts", mounts_matches))
		goto error;

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
	if (call_test(502))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
