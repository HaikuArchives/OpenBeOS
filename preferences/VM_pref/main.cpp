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
	
	FILE *settingsFile =
    	fopen("/boot/home/config/settings/kernel/drivers/virtual_memory", "r");
   	FILE *ptr;
    char dummy[80];
    BVolume bootVol;
    BVolumeRoster *vol_rost = new BVolumeRoster();
    
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
	
	fscanf(settingsFile, "%s %s\n", dummy, dummy);
	fscanf(settingsFile, "%s %d\n", dummy, &currSwap);
	fclose(settingsFile);
	currSwap = currSwap / 1048576;
	
	/* There's got to be a better way to get the amount of
	 * installed memory.
	 */
	if((ptr = popen("sysinfo -mem", "r")) != NULL){
	
		fscanf(ptr, "%d bytes free      (used/max %d / %d)", &physMem, &physMem, &physMem);
	
	}//if
	pclose(ptr);
	
	physMem = physMem / 1048576;
	minSwap = physMem + (int)(physMem / 3.0);
	
	vol_rost->GetBootVolume(&bootVol);
	/* maxSwap is defined by the amount of free space on your boot
	 * volume, plus the current swap file size, minus an arbitrary
	 * amount of space, just so you don't fill up your drive completly.
	 */
	maxSwap = (bootVol.FreeBytes() / 1048576) + currSwap - 16;
	
	MainWindowRect.Set(100, 80, 360, 300);
	Main = new MainWindow(MainWindowRect, physMem, currSwap, minSwap, maxSwap);
	
	Main->Show();

}

