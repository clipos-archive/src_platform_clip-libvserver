// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-vlogin.c 
 *  Terminal proxy functions for libclipvserver
 *  @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 *  Copyright (C) 2007 SGDN/DCSSI
 *  @n
 *  Largely based on util-vserver's vlogin.c, which is 
 *  Copyright (C) 2006 Benedikt Böhm <hollow@gentoo.org>,
 *  itself based on vserver-util's vlogin.
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <pty.h>
#include <utmp.h> /* login_tty */
#include <fcntl.h>

#include "clip-vserver.h"

/**
 * @internal
 * @brief Internal copy buffer size.
 */
#define BUF_LEN 256

/**
 * @internal
 * @brief Terminal proxy representation
 */
struct term {
	pid_t	pid; 	/**< pid for the proxy's slave session leader */
	int	fd;	/**< master fd for the proxy */
	int 	raw_p;	/**< 1 if master terminal has been set raw, 0 otherwise */
	struct termios oterm; /**< saved terminal settings before setting it raw */
};

/**
 * @internal
 * @brief Global terminal variable.
 */
static struct term g_term = {
	.pid = -1,
	.fd = -1,
	.raw_p = 0,
};

/**
 * @internal
 * @brief Simple write loop.
 */
static int
write_all(int fd, const char *buf, size_t len)
{
	ssize_t written;

	while (len > 0) {
		written = write(fd, buf, len);
		if (written < 0) {
			/* We don't deal with EAGAIN */
			if (errno == EINTR)
				continue;
			perror("write");
			return -1;
		}
		buf += written;
		len -= (size_t)written;
	}
	return 0;
}

/**
 * @internal
 * @brief Set the global terminal raw.
 * Used on the master process's terminal, to properly display the slave's
 * terminal output.
 * Non-reentrant. Can only be called once.
 */
static int
term_raw(void)
{
	struct termios term;

	if (g_term.raw_p) {
		fputs("Terminal has already been set raw\n", stderr);
		return -1;
	}

	if (tcgetattr(STDIN_FILENO, &g_term.oterm) == -1) {
		perror("tcgetattr");
		return -1;
	}
	(void)memcpy(&term, &g_term.oterm, sizeof(term));

	cfmakeraw(&term);

	g_term.raw_p = 1; /* We can set this as soon as oterm is valid */
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

/**
 * @internal
 * @brief Reset the master process's terminal to its original settings. 
 * Make it non-raw if it wasn't raw in the first place.
 * Only called through atexit(), we don't care about retvals 
 */
static void
term_reset(void)
{
	/* Only done by the father */
	if (g_term.pid == -1)
		return;
	if (!g_term.raw_p)
		return;
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_term.oterm) == -1)
		perror("tcsettatr original");
}

/**
 * @internal
 * @brief Redraw the proxy terminal.
 * Called initially, and each time the master process's terminal is redrawn.
 */
static int
term_redraw(void)
{
	struct winsize ws;
	
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1)
		return -1;

	if (ioctl(g_term.fd, TIOCSWINSZ, &ws) == -1)
		return -1;

	return kill(g_term.pid, SIGWINCH);
}

/**
 * @internal
 * @brief Basic copy of at most BUF_LEN characters from @a src_fd to @a dst_fd.
 * Normally called with blocking fds, except in term_end.
 */
static int
term_copy(int src_fd, int dst_fd)
{
	char buf[BUF_LEN];
	ssize_t len;

	len = read(src_fd, buf, sizeof(buf));
	if (len == -1) {
		if (errno == EINTR) {
			return 0;
		} else {
			if (errno != EIO) 
				perror("read");
			if (kill(g_term.pid, SIGTERM) == -1)
				perror("term_kill");
			/** Doesn't return an error for EIO on read : 
			 * this is expected behaviour * for slave termination, 
			 * when the read fails before we get the SIGCHLD */
			return (errno == EIO) ? 0 : -1;
		}
	}

	return write_all(dst_fd, buf, (size_t)len);
}

/**
 * @internal
 * @brief Attempt to flush the proxy's outputs before terminating it.
 * Inputs are not flushed. The proxy terminal is set non-blocking 
 * before flushing, to avoid delaying termination.
 */
static int
term_end(void)
{
	char buf[BUF_LEN];
	ssize_t len;
	long options;

	options = fcntl(g_term.fd, F_GETFL, 0);
	if (options == -1) {
		perror("fcntl GETFL");
		return -1;
	}
	options |= O_NONBLOCK;
	if (fcntl(g_term.fd, F_SETFL, options) == -1) {
		perror("fcntl SETFL");
		return -1;
	}

	/* Flush outputs from slave */
	for (;;) {
		len = read(g_term.fd, buf, sizeof(buf));
		if (!len || (len == -1 && (errno == EAGAIN || errno == EIO)))
			break;
		if (len == -1) {
			perror("read");
			return -1;
		}
		if (write_all(STDOUT_FILENO, buf, (size_t)len) == -1) 
			return -1;
	}

	return 0;
}

/**
 * @internal
 * @brief Signals handled by signals_handler().
 */
static const int const handled_sigs[] = { SIGINT, SIGTERM, SIGWINCH };

/**
 * @internal
 * @brief Saved signal handlers from before the call to clip_vlogin().
 */
static sighandler_t orig_sighandlers[sizeof(handled_sigs)/sizeof(int)];

/** 
 * @internal
 * @brief Saved SIGCHLD sigaction.
 */
static struct sigaction orig_sigchld_action;

/**
 * @internal
 * @brief Common signal handler for SIGINT, SIGTERM and SIGWINCH.
 * Transmits signals received by the  master process to the slave side.
 */
static void
signals_handler(int signum)
{
	/* This is only a valid sighandler in the father, do not run it 
	 * in the child */
	if (g_term.pid == -1)
		return;

	switch (signum) {
		case SIGINT:
		case SIGTERM:
			(void) kill(g_term.pid, signum);
			break;
		case SIGWINCH:
			(void) term_redraw();
			break;
		default:
			return;
	}
}

/**
 * @internal
 * @brief SIGCHLD handler.
 * Flushes the proxy before terminating its master process.
 * Only called on actual termination from the child process, not on 
 * stop/resume.
 */
static void
sigchld_handler(int signum)
{
	int status;

	/* This is only a valid sighandler in the father, do not run it 
	 * in the child */
	if (g_term.pid == -1)
		return;

	if (signum != SIGCHLD)
		return;

	(void) term_end();
	/* Clear zombies */
	if (waitpid(-1, &status, WNOHANG) == -1) 
		return; /* WTF ? We don't exit if our children don't */
	exit(WEXITSTATUS(status));
}

/**
 * @internal
 * @brief Set the SIGCHLD handler.
 * Makes sure sigchld_handler() is only called on actual child termination.
 */
static inline int
set_sigchld_handler(void)
{
	struct sigaction action;

	/* I don't really know what could happen here, but it does return 
	 * an int... */
	if (sigemptyset(&action.sa_mask) != 0)
		return -1;
	action.sa_flags = SA_NOCLDSTOP;
	action.sa_handler = sigchld_handler;

	if (sigaction(SIGCHLD, &action, &orig_sigchld_action) == -1) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

/** 
 * @internal
 * @brief Resets the signal handlers to their original value.
 * Called by the slave side to restore the callers signal handlers, 
 * and by the master side when returning.
 */
static inline int
reset_signals(void)
{
	unsigned int i;

	for (i = 0; i < sizeof(handled_sigs) / sizeof(*handled_sigs); i++) {
		if (orig_sighandlers[i] == SIG_ERR) {
			return -1;
		}
		if (signal(handled_sigs[i], orig_sighandlers[i]) == SIG_ERR) {
			perror("signal");
			return -1;
		}
	}

	if (sigaction(SIGCHLD, &orig_sigchld_action, NULL) == -1) {
		perror("signal");
		return -1;
	}

	return 0;
}

/**
 * @internal
 * @brief Child side code.
 * Closes the proxy's master side, and sets the slave as controlling tty, 
 * before returning control flow to the caller.
 * This also resets the custom SIGCHLD handler set up by the master process.
 * It does not return an error code, as error conditions result in an exit(),
 * to avoid returning an error to the caller after a successful fork (with a 
 * master process waiting).
 *
 * Note that there is a small race-condition before resetting the signal handler.
 * It is countered by the fact that the custom sighandler only does something 
 * in the father, but this means we can temporarily ignore some SIGCHLDs in the 
 * child.
 */
static void 
do_child(int slave)
{
	if (reset_signals()) {
		perror("reset_signals");
		exit(EXIT_FAILURE);
	}

	while (close(g_term.fd) == -1) {
		if (errno == EINTR)
			continue;
		perror("close master");
		exit(EXIT_FAILURE);
	}

	if (login_tty(slave) == -1) {
		perror("login_tty");
		exit(EXIT_FAILURE);
	}
}


static inline void die(int) __attribute__ ((noreturn));

/**
 * @internal
 * @brief Master side abnormal termination function.
 * Kills the child session and resets the original terminal before exiting.
 */
static inline void
die(int exit_code) 
{
	if (kill(g_term.pid, SIGTERM) == -1)
		perror("term_kill");
	term_reset();
	(void)reset_signals();
	exit(exit_code);
}

static void do_parent(int slave, pid_t pid) __attribute__ ((noreturn));

/**
 * @internal
 * @brief Master side main function.
 * Loops copying slave output to master stdout, and master stdin to slave input.
 * Loop is interrupted by either sigchld_handler() or signals_handler().
 */
static void
do_parent(int slave, pid_t pid)
{
	fd_set rfds;
	unsigned int i;
	sighandler_t orig;
	
	g_term.pid = pid;

	while (close(slave) == -1) {
		if (errno == EINTR)
			continue;
		perror("close slave");
		die(EXIT_FAILURE);
	}

	for (i = 0; i < sizeof(handled_sigs) / sizeof(*handled_sigs); i++) {
		orig_sighandlers[i] = SIG_ERR;
	}
	for (i = 0; i < sizeof(handled_sigs) / sizeof(*handled_sigs); i++) {
		orig = signal(handled_sigs[i], signals_handler);
		if (orig == SIG_ERR) {
			perror("signal");
			die(EXIT_FAILURE);
		}
		orig_sighandlers[i] = orig;
	}

	if (term_redraw() == -1) 
		/* Output error message, but try to go on */
		perror("term_redraw");

	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		FD_SET(g_term.fd, &rfds);

		while (select(g_term.fd + 1, &rfds, NULL, NULL, NULL) == -1) {
			if (errno == EINTR)
				continue;
			perror("select");
			die(EXIT_FAILURE);
		}
		if (FD_ISSET(STDIN_FILENO, &rfds))
			if (term_copy(STDIN_FILENO, g_term.fd) == -1)
				die((errno == EIO) ? EXIT_SUCCESS : EXIT_FAILURE);
		
		if (FD_ISSET(g_term.fd, &rfds))
			if (term_copy(g_term.fd, STDOUT_FILENO) == -1)
				die((errno == EIO) ? EXIT_SUCCESS : EXIT_FAILURE);

	}
	/* Not reached */
}


int
clip_vlogin(void)
{
	int slave;
	pid_t pid;

	if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
		return 0;
	
	g_term.pid = -1;
	g_term.fd = -1;
	g_term.raw_p = 0;
	/* Will do nothing if we're terminated before we set the
	 * terminal to raw */
	atexit(term_reset);

	if (term_raw() == -1)
		return -1;

	if (openpty(&g_term.fd, &slave, NULL, NULL, NULL) == -1) {
		perror("openpty");
		return -1;
	}

	if (set_sigchld_handler() == -1) 
		goto out_close;
	
	pid = fork();

	switch (pid) {
		case -1:
			perror("fork");
			goto out_close;
			break;
		case 0:
			do_child(slave);
			return 0;
			break;
		default:
			do_parent(slave, pid); /* no return */
			break;
	}

	/* We don't really fall through here... */
out_close:
	(void)close(g_term.fd);
	(void)close(slave);
	return -1;
}
