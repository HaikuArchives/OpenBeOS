/*! \file main.h
    \brief Header file for the main class.
    
*/

#ifndef MAIN_H
	
	#define MAIN_H
	
	#ifndef _APPLICATION_H
	
		#include <Application.h>
		
	#endif
	
	/**
	 * Main class.
	 *
	 * Gets everything going.
	 */
	class VM_pref : public BApplication{
	
		public:
		
			/**
			 * Constructor.
			 */
			VM_pref();
			
	};
	
#endif