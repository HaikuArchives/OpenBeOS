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
	#ifndef VM_SETTINGS_H
	
		#include "VMSettings.h"
		
	#endif
	
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
			VMSettings	*fSettings;
			
	};
	
#endif