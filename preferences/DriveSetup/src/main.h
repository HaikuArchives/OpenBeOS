/*! \file main.h
    \brief Header file for the main class.
    
*/

#ifndef MAIN_H
	
	#define MAIN_H
	
	#ifndef _APPLICATION_H
	
		#include <Application.h>
		
	#endif
	
	#ifndef _STDIO_H
	
		#include <stdio.h>
		
	#endif
	#ifndef POS_SETTINGS_H
	
		#include "PosSettings.h"
		
	#endif
	
	struct partition_info{
	
		int type;			/* partition type */
		char fs[255];		/* file system */
		char name[255];		/* volume name */
		char mount_pt[255];	/* mounted at */
		double size;		/* size (in MB) */
	
	};
	
	struct dev_info{
	
		char device[255];   /* the path to the device */
		bool map;			/* TRUE for intel, FALSE for apple */
		partition_info parts[4];	/* information on the partitions on the device */
		
	};
	
	/**
	 * Main class.
	 *
	 * Gets everything going.
	 */
	class DriveSetup : public BApplication{
	
		public:
		
			/**
			 * Constructor.
			 */
			DriveSetup();
			virtual ~DriveSetup();
		
		private:
			PosSettings	*fSettings;
			
	};
	
#endif