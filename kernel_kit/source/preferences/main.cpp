/*! \file main.cpp
    \brief Code for the main class.
    
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
	VM_pref vmApp;
	
	vmApp.Run();
	
	return(0);
	
}

/*
 * Constructor.
 * 
 * Provides a contstructor for the application.
 */
VM_pref::VM_pref()
			:BApplication("application/x-vnd.MSM-VirtualMemoryPrefPanel"){
	
	/*
	 * The main interface window.
	 */		
	MainWindow *Main;
	
	/*
	 * Sets the size for the main window.
	 */
	BRect MainWindowRect;
	
	/*
	 * The amount of physical memory in the machine.
	 */
	int physMem;
	
	/*
	 * The current size of the swap file.
	 */
	int currSwap;
	
	/*
	 * The minimum size the swap file can be.
	 */
	int minSwap;
	
	/*
	 * The maximum size the swap file can be.
	 */
	int maxSwap;
	
	/*
	 * Eventually these values will need to be set by some real
	 * method, but for now dummy values will work.
	 */
	physMem = 64;
	currSwap = 128;
	minSwap = 128;
	maxSwap = 5140;
	
	MainWindowRect.Set(100, 80, 360, 280);
	Main = new MainWindow(MainWindowRect, physMem, currSwap, minSwap, maxSwap);
	
	Main->Show();

}

