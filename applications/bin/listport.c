// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
//
//  Copyright (c) 2001-2002, OpenBeOS
//
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//
//  File:        listport.c
//  Author:      Daniel Reinhold (danielre@users.sf.net)
//  Description: lists all open ports in the system, organized by team
//
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

#include <stdio.h>
#include <stdlib.h>
#include <OS.h>


void list_team_ports  (team_id id);
void show_port_totals (void); 



int
main (int argc, char **argv)
	{
	//
	show_port_totals ();
	
	if (argc == 1)
		{
		// list for all teams
		int32     cookie = 0;
		team_info info;
		
		while (get_next_team_info (&cookie, &info) >= B_OK)
			{
			list_team_ports (info.team);
			}
		}
	else
		// list for each team_id on the command line
		while (--argc)
			{
			list_team_ports (atoi (*++argv));
			}
	
	return 0;
	}


void
show_port_totals ()
	{
	int32       max = 0, used = 0, left;
	system_info sys;
	
	if (get_system_info (&sys) == B_OK)
		{
		max  = sys.max_ports;
		used = sys.used_ports;
		}
	
	left = max - used;
	
	printf ("port: total: %5d, used: %5d, left: %5d\n", max, used, left);
	}


void
list_team_ports (team_id id)
	{	
	int32      cookie = 0;
	port_info  this_port;
	team_info  this_team;
	
	if (get_team_info (id, &this_team) == B_BAD_TEAM_ID)
		{
		printf ("\nteam %d unknown\n", id);
		return;
		}

	printf ("\nTEAM %4d (%s):\n", id, this_team.args);
	printf ("   ID                         name  capacity  queued\n");
	printf ("----------------------------------------------------\n");
	
	while (get_next_port_info (id, &cookie, &this_port) == B_OK)
		{
		printf ("%5d %28s  %8d  %6d\n",
		         this_port.port,
		         this_port.name,
		         this_port.capacity,
		         this_port.queue_count);
		}
	}

