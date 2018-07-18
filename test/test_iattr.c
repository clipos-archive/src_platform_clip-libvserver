// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  test_iattr.c - test for clip_set_iattr
 *  Copyright (C) 2007 SGDN/DCSSI
 *  Author: Vincent Strubel <clipos@ssi.gouv.fr>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 **/

#include "test_prelude.h"


#include <clip-vserver.h>
#include <linux/vserver/inode.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define ARGS_TYPE void
#define ARGS 
#define TEST_FUN spawn_context

static inline void 
print_args(ARGS_TYPE)
{
	printf("\nTesting set_iattr.\nResults:\n");
}

static int call_test(ARGS_TYPE);

static pid_t child_pid;
static int father_readfd;
static int father_writefd;

static void
sighandle(int sig)
{
	printf("\t\t%d got signal\n", getpid());
	exit((sig == SIGTERM) ? 0 : -1);
}

#define PROCFILE "/proc/filesystems"

#define wait4ack() do {\
	if (read(fd_read, &c, 1) != 1) { \
		perror("Child read"); \
		return -1; \
	} \
} while (0)

#define do_ack() do {\
	if (write(fd_write, "a", 1) != 1) { \
		perror("Child write"); \
		return -1; \
	} \
} while (0)

static int
do_child(int fd_read, int fd_write)
{
	char c;
	FILE *f;
	char buf[256];

	/* start */
	wait4ack();

	printf("\tChild trying to open %s...\n", PROCFILE);
	if ((f = fopen(PROCFILE, "r"))) {
		printf("\tChild managed to open %s as expected\n", PROCFILE);
		if (fgets(buf, 256, f) != NULL)
			printf("\tFirst line read:\n\t\t%s", buf);
		(void)fclose(f);
	} else {
		printf("\tChild could not open %s : %s\n", 
					PROCFILE, strerror(errno));
		return -1;
	}

	/* ack */
	do_ack();
		
	/* try 2 */
	wait4ack();
	
	printf("\tChild trying to open %s again...\n", PROCFILE);
	if ((f = fopen(PROCFILE, "r"))) {
		printf("\tChild managed to open %s!\n", PROCFILE);
		return -1;
	} else {
		printf("\tChild could not open %s as expected: %s\n", 
					PROCFILE, strerror(errno));
	}

	/* ack 2 */
	do_ack();

	return 0;
}

#undef wait4ack
#undef do_ack

static int
spawn_context(ARGS_TYPE)
{
	pid_t pid;
	int fds_childw[2];
	int fds_childr[2];

	if (pipe(fds_childw) == -1) {
		perror("pipe");
		return -1;
	}
	if (pipe(fds_childr) == -1) {
		perror("pipe");
		return -1;
	}
	pid = fork();
	switch (pid) {
		case -1: 
			perror("fork");
			return -1;
		case 0: 
			(void)close(fds_childw[0]);
			(void)close(fds_childr[1]);
			if (signal(SIGTERM, sighandle) == SIG_ERR) {
				perror("signal");
				exit(-1);
			}
			if (clip_new_context(500, 0, 0, 0)) {
				fputs("Context creation failed\n", stderr);
				exit(-1);
			}
			exit(do_child(fds_childr[0], fds_childw[1]));
		default:
			child_pid = pid;
			(void)close(fds_childw[1]);
			(void)close(fds_childr[0]);
			father_readfd = fds_childw[0];
			father_writefd = fds_childr[1];
			return 0;
	}
}
			
#define wait4ack() do {\
	if (read(father_readfd, &c, 1) != 1) { \
		perror("Father read"); \
		goto error; \
	} \
} while (0)

#define do_ack() do {\
	if (write(father_writefd, "a", 1) != 1) { \
		perror("Father write"); \
		goto error; \
	} \
} while (0)

static int
show_results(void)
{
	char c;
	int status;
	printf("\tFather setting ~HIDE on %s\n", PROCFILE);
	if (clip_set_iattr(PROCFILE, 0, IATTR_HIDE)) {
		fputs("\tset_iattr failed\n", stderr);
		goto error;
	}
	
	do_ack();
	wait4ack();
	
	printf("\tFather setting HIDE on %s\n", PROCFILE);
	if (clip_set_iattr(PROCFILE, IATTR_HIDE, 0)) {
		fputs("\tset_iattr failed\n", stderr);
		goto error;
	}
	
	do_ack();
	wait4ack();
	
	if (waitpid(child_pid, &status, 0) != child_pid) {
		fputs("WTF???\n", stderr);
		goto error;
	}
	if (!WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
error:
	if(kill(child_pid, SIGTERM))
		perror("kill");
	return -1;
}

int 
main(void)
{
	if (call_test())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

#include "test_common.h"
