// SPDX-License-Identifier: GPL-2.0
// Copyright © 2007-2018 ANSSI. All Rights Reserved.
/**
 *  @file clip-vserver.h  Main header for libclipvserver
 *  @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 *  Copyright 2007-2009 SGDN/DCSSI
 *  Copyright 2010-2014 SGDSN/ANSSI
 *  @n
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License 
 *  version 2 as published by the Free Software Foundation.
 */

#ifndef _CLIP_VSERVER_H
#define _CLIP_VSERVER_H

#include <stdint.h>
#include <sys/types.h>

/**
 * @mainpage clip-libvserver documentation
 * This is the inline documentation for the clip-libvserver package.
 * @n
 * Public interface documentation is available directly through the 
 * clip-vserver.h reference.
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 */

/**
 * Security context id.
 */
typedef uint32_t xid_t;
/**
 * Network context id.
 */
typedef uint32_t nid_t;
/**
 * Context capabilities.
 */
typedef uint_least64_t vs_caps;
/**
 * Context flags.
 */
typedef uint_least64_t vs_flags;


/**
 * Create a new set of namespaces and execute in it.
 * This clones a new thread, with new namespaces for IPC, utsname and VFS. 
 * On success, execution continues in * the cloned thread, while the original 
 * thread is left waiting. The original thread will exit as soon 
 * as the child thread exits, setting its exit code accordingly.
 * The following namespaces are cloned : VFS, UTS, USER, IPC.
 * The function unconditionnally resets SIGCHLD handler to SIG_DFL.
 * @n @b Non-reentrant.
 * @return 0 on success (in the child thread), -1 on error.
 */
extern int clip_new_namespace(void);

/** Clone PID namespace in @a clip_new_namespace_opts */
#define CLIP_VSERVER_PIDNS 0x01 
/** Clone NET namespace in @a clip_new_namespace_opts */
#define	CLIP_VSERVER_NETNS 0x02
/** Clone USER namespace in @a clip_new_namespace_opts */
#define	CLIP_VSERVER_USRNS 0x04

/**
 * Create a new user-defined set of namespaces and execute in it.
 * This is the same as @a clip_new_namespace(), except it can also 
 * clone optional types of namespaces (namely PID, NET and / or USER) on
 * top of those cloned by @a clip_new_namespace().
 * @param opts Optional namespaces to create, as a bitwise OR of 0,
 * CLIP_VSERVER_PIDNS, CLIP_VSERVER_USRNS and CLIP_VSERVER_NETNS.
 * @n @b Non-reentrant.
 * @return 0 on success (in the child thread), -1 on error.
 */
extern int clip_new_namespace_opts(int opts);

/** 
 * Prototype of functions that can be called by 
 * @clip_new_namespace_callback to perform configuration operations in
 * a set of newly created namespaces.
 * @param pid Pid of a process running in the new namespaces, can be
 * used to designate these namespaces.
 * @param data Implementation defined data.
 * @return 0 on success, -1 on error.
 */
typedef int (*clip_ns_callback_t)(pid_t pid, void *data);

/**
 * Create a new user-defined set of namespaces and execute in it, while
 * also running a setup callback outside of those namespaces to perform
 * specific configuration operations.
 * This is the same as @clip_new_namespace, except that a caller defined
 * callback is run outside the new namespaces, after they have been created.
 * This can be used to e.g. move a virtual interface in a new network
 * namespace.
 * @param opts Optional namespaces to create, as a bitwise OR of 0,
 * CLIP_VSERVER_PIDNS, CLIP_VSERVER_USRNS and CLIP_VSERVER_NETNS.
 * @param cb Callback to run outside the new namespaces after their creation.
 * @param data Caller-defined data to pass as argument to the callback.
 * @n @b Non-reentrant.
 * @return 0 on success (in the child thread), -1 on error.
 */
extern int clip_new_namespace_callback(int opts, 
			clip_ns_callback_t cb, void *data);

/** 
 * Create a new security context (with private namespaces) and execute in it.
 * This creates a new security context with xid @a xid, associates it with the 
 * namespaces (IPC, VFS, utsname) of the caller, and sets it up with 
 * capabilities @a bcaps and @a ccaps, and flags @a cflags. 
 * @n @b Non-reentrant.
 * @param xid Context id for the new security context.
 * This id must not be associated with an existing context.
 * @param bcaps Authorized POSIX capabilities mask for the new context.
 * @param ccaps Vserver capabilities mask for the new context.
 * @param cflags Vserver flags for the new context. 
 * @return 0 on success, -1 on error.
 */
extern int clip_new_context(xid_t xid, vs_caps bcaps, 
				vs_caps ccaps, vs_flags cflags);
/** 
 * Create a new security context (with configurable private namespaces) 
 * and execute in it.
 * This is the same as @a clip_new_context(), except for the nsopts
 * parameter which allows the caller to specify which of its optional 
 * namespaces (PID, USER, NET) will be copied into the context, in 
 * a manner similar to @a clip_new_namespace_opts().
 * @n @b Non-reentrant.
 * @param xid Context id for the new security context.
 * This id must not be associated with an existing context.
 * @param bcaps Authorized POSIX capabilities mask for the new context.
 * @param ccaps Vserver capabilities mask for the new context.
 * @param cflags Vserver flags for the new context. 
 * @param opts Optional namespaces to copy, as a bitwise OR of 0,
 * CLIP_VSERVER_PIDNS, CLIP_VSERVER_USRNS and CLIP_VSERVER_NETNS.
 * @return 0 on success, -1 on error.
 */
extern int clip_new_context_nsopts(xid_t xid, vs_caps bcaps, vs_caps ccaps, 
					vs_flags flags, int nsopts);
/**
 * Enter an existing jail (security + network contexts and namespaces).
 * This puts the calling process in an existing vserver jail, by entering
 * its security and network contexts, and its various namespaces. 
 * @n @b Non-reentrant.
 * @param xid Context id for both the security and network contexts. 
 * This id must be associated with an existing context.
 * @return 0 on success, -1 on error.
 */
extern int clip_enter_context(xid_t xid);


/**
 * Enter an existing jail (security + network contexts, but not namespaces).
 * This puts the calling process in an existing vserver jail, by entering
 * its security and network contexts. This however does not enter the various
 * namespaces of the jail. This function is intended for use in cases where 
 * the caller has already entered those namespaces.
 * @n @b Non-reentrant.
 * @param xid Context id for both the security and network contexts. 
 * This id must be associated with an existing context.
 * @return 0 on success, -1 on error.
 */
extern int clip_enter_context_nons(xid_t xid);

/**
 * Enter an existing security context.
 * This puts the calling process in an existing vserver security 
 * contetx, without entering the corresponding namespaces or network
 * context.
 * @n @b Non-reentrant.
 * @param xid Context id for both the security and network contexts. 
 * This id must be associated with an existing context.
 * @return 0 on success, -1 on error.
 */
extern int clip_enter_security_context(xid_t xid);


/** 
 * Enter all namespaces associated with a security context.
 * This puts the calling process in the various namespaces associated with 
 * an existing security context, without entering the context itself.
 * @n @b Non-reentrant.
 * @param xid Context id for the target security context.
 * This id must be associated with an existing context.
 * @return 0 on success, -1 on error.
 */
extern int clip_enter_namespace(xid_t xid);

/** 
 * Enter some of the namespaces associated with a security context.
 * This puts the calling process in some of the various namespaces 
 * associated with * an existing security context, without entering the 
 * context itself.
 * @n @b Non-reentrant.
 * @param xid Context id for the target security context.
 * This id must be associated with an existing context.
 * @param mask Set of clone(2) flags defining which namespaces to enter.
 * @return 0 on success, -1 on error.
 */
extern int clip_enter_some_namespaces(xid_t xid, uint64_t mask);

/**
 * Create a new net context bound to a single address and execute in it.
 * This creates a new network context, and attributes a single address and
 * netmask to it. When successful, execution of the caller continues in the
 * new context.
 * @n @b Non-reentrant.
 * @deprecated This function is deprecated, use clip_new_net_context_multi()
 * instead.
 * @param nid Context id for the new network context. 
 * This id must not be associated with an existing context.
 * @param addr IP address for the new context, in number-and-dots format, e.g.
 * @a 192.168.0.1.
 * @param mask Netmask for the new context's address, in number-and-dots 
 * format.
 * @param nflags Network context flags for the new context.
 * @return 0 on success, -1 on error.
 */
extern int clip_new_net_context(nid_t nid, const char *addr, 
					const char *mask, vs_flags nflags);
/**
 * Create a new net context bound to multiple addresses and execute in it.
 * This creates a new network context, and attributes a list of addresses and
 * netmasks to it (to a maximum of 4 each). 
 * When successful, execution of the caller continues in the
 * new context.
 * @n @b Non-reentrant.
 * @param nid Context id for the new network context. 
 * This id must not be associated with an existing context.
 * @param addrs IP addresses for the new context, specified as an array of
 * at most 4 strings. If less than 4 strings are passed, the array must contain
 * a NULL pointer after the last significant string. Each string defines an 
 * address in number-and-dots format.
 * @param masks Netmasks for the new context's addresses, specified in the same
 * way as @a addrs. There must be as many masks as addresses. Each mask is 
 * associated with the address at the same offset in @a addrs.
 * @param nflags Network context flags for the new context.
 * @return 0 on success, -1 on error.
 */
extern int clip_new_net_context_multi(nid_t nid, const char **addrs, 
					const char **masks, vs_flags nflags);

/**
 * Add addresses to an existing network context.
 * @param nid Context id of the target network context.
 * @param addrs NULL-terminated list of (at most 4) IP addresses, 
 * in number and dots format.
 * @param masks NULL-terminated list of netmasks, in number and dots 
 * format and in the same order as the addresses in @a addrs.
 * @return 0 on success, -1 on error.
 */
extern int clip_net_add_addrs(nid_t nid, 
	const char **addrs, const char **masks);

/**
 * Set the broadcast address for a network context.
 * @param nid Context id of the target network context.
 * @param addr Broadcast address, in number and dot format.
 * @return 0 on success, -1 on error.
 */
extern int clip_net_set_bcast(nid_t nid, const char *addr);

/**
 * Set the loopback address for a network context.
 * @param nid Context id of the target network context.
 * @param addr Loopback address, in number and dot format.
 * @return 0 on success, -1 on error.
 */
extern int clip_net_set_lback(nid_t nid, const char *addr);


/**
 * Remove all addresses from a network context.
 * @param nid Context id of the target network context.
 * @return 0 on success, -1 on error.
 */
extern int clip_net_del_addrs(nid_t nid);

/** 
 * Create a new full vserver jail and execute in it.
 * This creates a simple vserver jail, namely a new set of namespaces,
 * a new root, and new network and security contexts.
 * The network context is created with a single IP address @a addr, 
 * with "255.255.255.0" as the associated netmask, and no context flags.
 * The security context is created with the POSIX capabilities @a bcaps,
 * the vserver flags @a cflags, and no vserver capabilities. @a root is 
 * used as the root of the new namespace, which is otherwise a copy of 
 * the caller's namespace.
 * @n
 * This call is in essence a shortcut for calling clip_new_net_context(),
 * clip_new_namespace(), mount()/chroot(), and clip_new_context(), with 
 * default arguments.
 * @n @b Non-reentrant.
 * @param xid Context id of both new contexts (security and network).
 * This id must not be associated with an existing context.
 * @param bcaps Authorized POSIX capabilities mask in the new security context.
 * @param cflags Vserver flags for the new security context.
 * @param addr IP address for the new network context.
 * @param path Path to the new VFS namespace's root.
 * @return 0 on success, -1 on error.
 */
extern int clip_jailself(xid_t xid, vs_caps bcaps, vs_flags cflags, 
				const char * addr, const char *path);

/**
 * Setup a terminal proxy. 
 * This creates a pseudo-terminal, and forks a child process that uses the 
 * pseudo-tty slave fd as its controlling terminal, closing the original 
 * controlling terminal. Meanwhile, the parent process copies every output
 * from the master fd to its standard output, and every one of its standard 
 * inputs to that same master fd, effectively proxying the child's STDIN/STDOUT 
 * to its own. The master process exits when the child exits.
 * @n @b Non-reentrant.
 * @return @li 0 on sucess (returned in the child - the function never returns 
 * in the parent) @li -1 on error (in the parent - no child is created in that 
 * case).  @li Note that in some cases, the whole function can directly exit 
 * in both the parent and the child, without ever returning - this happens 
 * if the child code fails after the fork but before returning.
 */
extern int clip_vlogin(void);

/**
 * Set inode attributes.
 * This sets or unset a mask of vserver inode attributes 
 * (IATTR_HIDE/IATTR_ADMIN...) on a file @a path.
 * Returns an error if a bit is set in both @a set and @a unset.
 * @param path Path of the target file.
 * @param set Mask of attributes to set.
 * @param unset Mask of attributes to unset. 
 * @return 0 on success, -1 on error.
 */
extern int clip_set_iattr(const char *path, uint32_t set, uint32_t unset);

#endif /* _CLIP_VSERVER_H */
