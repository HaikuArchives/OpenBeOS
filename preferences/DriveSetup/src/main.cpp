/*! \file main.cpp
 *  \brief Code for the main class.
 *  
 *  This file contains the code for the main class.  This class sets up all
 *  of the initial conditions for the app.
 *
*/

#ifndef MAIN_WINDOW_H

	#include "MainWindow.h"
	
#endif
#ifndef MAIN_H

	#include "main.h"
	
#endif

/**
 * Main method.
 * 
 * Starts the whole thing.
 */
int main(int, char**){

	/**
	 * An instance of the application.
	 */
	DriveSetup dsApp;
	
	dsApp.Run();
	
	return(0);
	
}

/*
 * Constructor.
 * 
 * Provides a contstructor for the application.
 */
DriveSetup::DriveSetup()
			:BApplication("application/x-vnd.MSM-DriveSetupPrefPanel"){
	
	fSettings = new VMSettings();
    
	/*
	 * The main interface window.
	 */		
	MainWindow *Main;
	
	Main = new MainWindow(fSettings->WindowPosition(), fSettings);
	
	Main->Show();

}


DriveSetup::~DriveSetup()
{
		delete fSettings;
}

