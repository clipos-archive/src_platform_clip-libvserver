# SPDX-License-Identifier: GPL-2.0
# Copyright © 2007-2018 ANSSI. All Rights Reserved.
clipincludedir = $(includedir)/clip
clipinclude_HEADERS = \
	clip-vserver.h

lib_LTLIBRARIES = libclipvserver.la

libclipvserver_la_SOURCES = \
	clip-namespace.c \
	clip-context.c \
	clip-vlogin.c \
	clip-net.c \
	clip-iattr.c

libclipvserver_la_CFLAGS = $(CFLAGS)

if NET_NS
libclipvserver_la_CFLAGS += -DNET_NS
endif

libclipvserver_la_LDFLAGS = -version-info @VERSION_INFO@ -lutil

INCLUDES = -I$(srcdir) -idirafter @KERNEL_INCLUDES@ ${CLIP_CFLAGS} 

