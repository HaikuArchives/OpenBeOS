// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
//
//  Copyright (c) 2001-2002, OpenBeOS
//
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//
//  File:        hd.c
//  Author:      Daniel Reinhold (danielre@users.sf.net)
//  Description: hex dump utility
//
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

#include <OS.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>


void do_hd      (char *);
void dump_file  (FILE *);
void display    (uint32, uint8 *);
char *hexbytes  (uint8 *);
char *printable (uint8 *);
void usage      ();


int BytesBetweenSpaces = 1;


void
usage ()
	{
	printf ("usage:\thd [-n N] [file]\n");
	puts ("\t-n expects a number between 1 and 16 and specifies");
    puts ("\tthe number of bytes between spaces.");
	puts ("");
    puts ("\tIf no file is specified, input is read from stdin");
	}


int
main (int argc, char *argv[])
	{
	char *arg = NULL;
	
	if (argc == 1)
		{
		dump_file (stdin);
		}
	else
		{
		char *first = *++argv;
		
		if (strcmp (first, "--help") == 0)
			{
			usage ();
			return 0;
			}
		
		if (strcmp (first, "-n") == 0)
			{
			if (--argc > 1)
				{
				char *num = *++argv;
				
				if (!isdigit (*num))
					printf ("-n option needs a numeric argument\n");
				else
					{
					int b = atoi (num);
					
					if (b < 1)  b = 1;
					if (b > 16) b = 16;
					BytesBetweenSpaces = b;
					
					if (--argc > 1)
						arg = *++argv;
					else
						printf ("no file specified\n");
					}
				}
			else
				printf ("-n option needs a numeric argument\n");
			}
		else
			arg = first;
		
		if (arg)
			do_hd (arg);
		}
	
	putchar ('\n');
	return 0;
	}


void
do_hd (char *fname)
	{
	struct stat e;
	
	if (stat (fname, &e) == -1)
		{
		fprintf (stderr, "'%s': no such file or directory\n", fname);
		return;
		}

	if (S_ISDIR (e.st_mode))
		fprintf (stderr, "'%s' is a directory\n", fname);
	else
		{
		FILE *fp = fopen (fname, "rb");
		if (fp)
			{
			dump_file (fp);
			fclose (fp);
			}
		else
			fprintf (stderr, "'%s': %s\n", fname, strerror (errno));
		}
	}


void
dump_file (FILE *fp)
	{
	size_t got;
	uint32 offset = 0;
	uint8  data[16];
	
	while ((got = fread (data, 1, 16, fp)) == 16)
		{
		display (offset, data);
		offset += 16;
		}
	
	if (got > 0)
		{
		memset (data+got, ' ', 16-got);
		display (offset, data);
		}
	}


void
display (uint32 offset, uint8 *data)
	{
	printf ("%08x ", offset);
	printf ("  %s ", hexbytes  (data));
	printf ("%16s ", printable (data));
	
	putchar ('\n');
	}


char *
hexbytes (uint8 *s)
	{
	static char buf[64];
	char *p = buf;
	uint8 c;
	int   i;
	int   n = 0;
	
	for (i = 0; i < 16; ++i)
		{
		c = *s++;
		*p++ = "0123456789abcdef"[c/16];
		*p++ = "0123456789abcdef"[c%16];
		
		if (++n == BytesBetweenSpaces)
			{
			*p++ = ' ';
			n = 0;
			}
		
		if ((i == 7) && (BytesBetweenSpaces == 1))
			*p++ = ' ';
		}
	*p++ = ' ';
	*p = 0;
	
	return buf;
	}


char *
printable (uint8 *s)
	{
	static char buf[16];
	int   n = 16;
	char *p = buf;
	uint8 c;
	
	while (n--)
		{
		c = *s++;
		*p++ = (isgraph (c) ? c : '.');
		}
	*p = 0;
	
	return buf;
	}
