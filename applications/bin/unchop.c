// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
//
//  Copyright (c) 2001-2002, OpenBeOS
//
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//
//  File:        unchop.c
//  Author:      Daniel Reinhold (danielre@users.sf.net)
//  Description: recreates a file previously split with chop
//
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

#include <OS.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>


void  usage       (void);
void  do_unchop   (char *, char *);
void  concatenate (FILE *, FILE *);
bool  valid_file  (char *);
void  replace     (char *, char *);
char *temp_file   ();



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// globals

#define BLOCKSIZE 64000         // file data is read in BLOCKSIZE blocks
static char Block[BLOCKSIZE];   // and stored in the global Block array

int Errors = 0;



void
usage ()
	{
	puts ("usage: unchop file");
	puts ("Concatenates files named file00, file01... into file");
	}


int
main (int argc, char *argv[])
	{	
	if (argc != 2)
		{
		usage ();
		return 0;
		}
	else
		{
		char *origfile = argv[1];
		char *tmpfile  = origfile;
		bool  needs_replace = false;
	
		if (valid_file (origfile))
			{
			// output file already exists -- write to temp file
			tmpfile = temp_file ();
			needs_replace = true;
			}
		
		do_unchop (tmpfile, origfile);
		
		if (needs_replace)
			{
			if (Errors == 0)
				replace (origfile, tmpfile);
			else
				remove (tmpfile);
			}
		}
	
	putchar ('\n');
	return Errors;
	}


void
do_unchop (char *outfile, char *basename)
	{
	FILE *fout = fopen (outfile, "ab");
	if (fout)
		{
		int  i;
		char fnameN[256];
		
		for (i = 0; i < 999999; ++i)
			{
			sprintf (fnameN,  "%s%02d", basename, i);
			
			if (valid_file (fnameN))
				{
				FILE *fin = fopen (fnameN, "rb");
				if (fin)
					{
					concatenate (fin, fout);
					fclose (fin);
					}
				else
					{
					fprintf (stderr, "'%s': %s\n", fnameN, strerror (errno));
					++Errors;
					}
				}
			else
				{
				if (i == 0)
					printf ("No chunk files present (%s)", fnameN);
					
				// run out of chunk files to concatenate
				break;
				}
			}
		fclose (fout);
		}
	else
		fprintf (stderr, "'%s': %s\n", outfile, strerror (errno));
	}


void
concatenate (FILE *fin, FILE *fout)
	{
	int got;
	
	for (;;)
		{
		got = fread (Block, 1, BLOCKSIZE, fin);
		if (got == 0)
			break;
		
		fwrite (Block, 1, got, fout);
		}
	}


bool
valid_file (char *fname)
	{
	// for this program, a valid file is one that:
	//   a) exists (that always helps)
	//   b) is a regular file (not a directory, link, etc.)
	
	struct stat e;
		
	if (stat (fname, &e) == -1)
		{
		// no such file
		return false;
		}

	return (S_ISREG (e.st_mode));
	}


void
replace (char *origfile, char *newfile)
	{
	// replace the contents of the original file
	// with the contents of the new file
	
	char buf[1000];
	
	// delete the original file
	remove (origfile);
	
	// rename the new file to the original file name
	sprintf (buf, "mv \"%s\" \"%s\"", newfile, origfile);
	system (buf);
	}


char *
temp_file ()
	{
	char *tmp = tmpnam (NULL);
	
	FILE *fp = fopen (tmp, "w");
	fclose (fp);
	
	return tmp;
	}
