/*! \file structs.h
    \brief Holds the structs used.
    
*/

#ifndef STRUCTS_H

	#define STRUCTS_H
	
	struct partition_info{
	
		int type;			/* partition type */
		char fs[255];		/* file system */
		char name[255];		/* volume name */
		char mount_pt[255];	/* mounted at */
		double size;		/* size (in MB) */
	
	};
	
	struct dev_info{
	
		char *device;   /* the path to the device */
		bool map;			/* TRUE for intel, FALSE for apple */
		partition_info parts[4];	/* information on the partitions on the device */
		
	};
	
#endif

