# SPDX-License-Identifier: GPL-2.0
# Copyright © 2007-2018 ANSSI. All Rights Reserved.
TESTS_SRC := test_context.c \
		test_context_enter.c \
		test_net.c \
		test_net_multi.c \
		test_vlogin.c \
		test_namespace.c \
		test_namespace_enter.c \
		test_jail.c \
		test_iattr.c

LIBS := clipvserver util

###############################################################################

ifndef VERBOSE
MAKEFLAGS += -s --no-print-directory
ECHO = echo
else
ECHO = echo > /dev/null
endif

LIBFLAGS := ${foreach lib, ${LIBS}, -l${lib}}
CFLAGS := -Wall -I../src -Wno-unused-function -I/usr/src/linux/include
LDFLAGS := -L../src/.libs -Wl,-rpath,${PWD}/../src/.libs ${LIBFLAGS}

TESTS := ${patsubst %.c,%.test, ${TESTS_SRC}}

all: ${TESTS}

%.test:%.c Makefile
	@${ECHO} "        CC                $<"
	$(CC) -o $@ $< ${CFLAGS} ${LDFLAGS}

clean: 
	@${ECHO} "        CLEAN"
	rm -f ${TESTS}

