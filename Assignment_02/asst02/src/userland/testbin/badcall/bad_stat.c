/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Bad calls to fstat, lstat, and stat
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "config.h"
#include "test.h"

////////////////////////////////////////////////////////////

static
int
badbuf_fstat(struct stat *sb)
{
	return fstat(STDIN_FILENO, sb);
}

static
int
badbuf_lstat(struct stat *sb)
{
	return lstat("null:", sb);
}

static
int
badbuf_stat(struct stat *sb)
{
	return stat("null:", sb);
}

static
void
common_badbuf(int (*statfunc)(struct stat *), void *ptr,
	      const char *call, const char *ptrdesc)
{
	int rv;

	report_begin("%s with %s buf", call, ptrdesc);
	rv = statfunc(ptr);
	report_check(rv, errno, EFAULT);
}

static
void
any_badbuf(int (*statfunc)(struct stat *), const char *call)
{
	common_badbuf(statfunc, NULL, call, "NULL");
	common_badbuf(statfunc, INVAL_PTR, call, "invalid pointer");
	common_badbuf(statfunc, KERN_PTR, call, "kernel pointer");
}

////////////////////////////////////////////////////////////

static
void
any_empty(int (*statfunc)(const char *, struct stat *), const char *call)
{
	struct stat sb;
	int rv;

	report_begin("%s on empty string", call);
	rv = statfunc("", &sb);
	report_check2(rv, errno, 0, EINVAL);
}

////////////////////////////////////////////////////////////

void
test_fstat(void)
{
	test_fstat_fd();
	any_badbuf(badbuf_fstat, "fstat");
}

void
test_lstat(void)
{
	test_lstat_path();
	any_empty(lstat, "lstat");
	any_badbuf(badbuf_lstat, "lstat");
}

void
test_stat(void)
{
	test_stat_path();
	any_empty(stat, "stat");
	any_badbuf(badbuf_stat, "stat");
}

