/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAXALIASES	35

static FILE *protof = NULL;
static char line[BUFSIZ+1];
static struct protoent proto;
static char *proto_aliases[MAXALIASES];
int _proto_stayopen;

void setprotoent(int f)
{
	if (protof == NULL)
		protof = fopen(_PATH_PROTOCOLS, "r" );
	else
		rewind(protof);
	_proto_stayopen |= f;
}

void endprotoent()
{
	if (protof) {
		fclose(protof);
		protof = NULL;
	}
	_proto_stayopen = 0;
}

struct protoent *getprotoent()
{
	char *p, *cp, **q, *endp;
	long l;
	size_t len;
	char buf[80];

	if (protof == NULL && (protof = fopen(_PATH_PROTOCOLS, "r" )) == NULL)
		return (NULL);
again:
	if (fgets(buf, 80, protof) == NULL)
		return NULL;
	len = strlen(buf);
	p = buf;
	
//	if ((p = fgetln(protof, &len)) == NULL)
//		return (NULL);
	if (p[len-1] == '\n')
		len--;
	if (len >= sizeof(line) || len == 0)
		goto again;
	p = memcpy(line, p, len);
	line[len] = '\0';
	if (*p == '#')
		goto again;
	if ((cp = strchr(p, '#')) != NULL)
		*cp = '\0';
	proto.p_name = p;
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = strpbrk(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	l = strtol(cp, &endp, 10);
	if (endp == cp || *endp != '\0' || l < 0 || l >= INT_MAX)
		goto again;
	proto.p_proto = l;
	q = proto.p_aliases = proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &proto_aliases[MAXALIASES - 1])
				*q++ = cp;
			cp = strpbrk(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (&proto);
}
