// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
//
//  Copyright (c) 2001-2002, OpenBeOS
//
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//
//  File:        chop.c
//  Author:      Daniel Reinhold (danielre@users.sf.net)
//  Description: splits one file into a collection of smaller files
//
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

#include <OS.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>


void usage      (void);
void do_chop    (char *);
void chop_file  (FILE *, char *, int);



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// globals

#define BLOCKSIZE 64000         // file data is read in BLOCKSIZE blocks
static char Block[BLOCKSIZE];   // and stored in the global Block array

int KBytesPerChunk = 1400;      // determines size of output files


void
usage ()
	{
	puts ("usage: chop [-n kbyte_per_chunk] file");
	puts ("Splits file into smaller files named file00, file01...");
    puts ("Default split size is 1400k");
	}



int
main (int argc, char *argv[])
	{
	char *arg = NULL;
	char *first;
	
	if ((argc < 2) || (argc > 4))
		{
		usage ();
		return 0;
		}
	
	first = *++argv;
	
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
				KBytesPerChunk = b;
				
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
		do_chop (arg);
	
	putchar ('\n');
	return 0;
	}


void
do_chop (char *fname)
	{
	struct stat e;
	off_t  fsize;
	
	if (stat (fname, &e) == -1)
		{
		fprintf (stderr, "'%s': no such file or directory\n", fname);
		return;
		}

	if (S_ISDIR (e.st_mode))
		{
		fprintf (stderr, "'%s' is a directory\n", fname);
		return;
		}
	
	fsize = e.st_size;
	if (fsize < KBytesPerChunk * 1024)
		{
		fprintf (stderr, "'%s': file is already small enough\n", fname);
		}
	else
		{
		FILE *fp = fopen (fname, "rb");
		if (fp)
			{			
			chop_file (fp, fname, fsize);
			fclose (fp);
			}
		else
			fprintf (stderr, "'%s': %s\n", fname, strerror (errno));
		}
	}


void
chop_file (FILE *fin, char *fname, int fsize)
	{
	const int chunk_size = KBytesPerChunk * 1024;  // max bytes written to any output file
	
	bool open_next_file = true;  // when to open a new output file
	char fnameN[256];            // name of the current output file (file01, file02, etc.)
	int  index = 0;              // used to generate the next output file name
	FILE *fout;                  // output file pointer

	int  total_written = 0;      // total bytes read from input file that have been written out
	int  got;                    // size of the current data block -- i.e. from the last fread()
	int  put;                    // number of bytes just written   -- i.e. from the last fwrite()
	int  needed;                 // how many bytes we can safely write to the current output file
	int  avail;                  // how many bytes we can safely grab from the current data block
	int  curr_written;           // total bytes written to the current output file
		
	char *beg = Block;  // pointer to the beginning of the block data to be written out
	char *end = Block;  // end of the current block (init to beginning to force first block read)
	
	printf ("Chopping up %s into %d kbyte chunks\n", fname, KBytesPerChunk);
	
	while (total_written < fsize)
		{
		if (beg >= end)
			{
			// read in another block
			got = fread (Block, 1, BLOCKSIZE, fin);
			if (got == 0)
				break;
			
			beg = Block;
			end = Block + got - 1;
			}
		
		if (open_next_file)
			{
			// start a new output file
			sprintf (fnameN,  "%s%02d", fname, index++);
			fout = fopen (fnameN, "w");
			if (!fout)
				{
				fprintf (stderr, "unable to write to file '%s': %s\n", strerror (errno));
				return;
				}
			curr_written = 0;
			open_next_file = false;
			}
		
		needed = chunk_size - curr_written;
		avail  = end - beg + 1;
		if (needed > avail)
			needed = avail;

		if (needed > 0)
			{
			put = fwrite (beg, 1, needed, fout);
			beg += put;
			}
		
		curr_written  += put;
		total_written += put;
		
		if (curr_written >= chunk_size)
			{
			// the current output file is full
			fclose (fout);
			open_next_file = true;
			}
		}
	
	// close up the last output file
	fclose (fout);
	}
