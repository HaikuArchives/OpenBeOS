/* 
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <libc/string.h>
#include <libc/ctype.h>

char *strstr(const char *s1, const char *s2)
{
	int l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = strlen(s1);
	while(l1 >= l2) {
		l1--;
		if (!memcmp(s1,s2,l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}
