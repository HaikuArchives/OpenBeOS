/*! \file MainWindow.cpp
 *  \brief Code for the MainWindow class.
 *  
 *  Displays the main window, the essence of the app.
 *
*/

#ifndef MAIN_WINDOW_H

	#include "MainWindow.h"
	
#endif

/**
 * Constructor.
 * @param frame The size to make the window.
 */	
MainWindow::MainWindow(BRect frame, PosSettings *Settings)
			:BWindow(frame, "DriveSetup", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE){

	BRect bRect;
	BMenu *menu;
	BMenu *tmpMenu;
	fSettings = Settings;
	
	bRect = Bounds();
	bRect.bottom = 18.0;
	
	rootMenu = new BMenuBar(bRect, "root menu");
	//Setup Mount menu
	menu = new BMenu("Mount");
	menu->AddItem(new BMenuItem("Mount All Partitions", NULL, 'M'));
	//add menu items for list of partitions
	rootMenu->AddItem(menu);
	
	//Setup Unmount menu
	menu = new BMenu("Unmount");
	//add menu items for list of partitions
	rootMenu->AddItem(menu);
	
	//Setup Setup menu
	menu = new BMenu("Setup");
	menu->AddItem(new BMenuItem("Format", NULL, 'F'));
	tmpMenu = new BMenu("Partition");
	tmpMenu->AddItem(new BMenuItem("apple...", NULL));
	tmpMenu->AddItem(new BMenuItem("intel...", NULL));
	menu->AddItem(tmpMenu);
	tmpMenu = new BMenu("Initialize");
	//add menu items for list of partitions
	menu->AddItem(tmpMenu);
	
	rootMenu->AddItem(menu);
	
	//Setup Options menu
	menu = new BMenu("Options");
	menu->AddItem(new BMenuItem("Eject", NULL, 'E'));
	menu->AddItem(new BMenuItem("Surface Test", NULL, 'T'));
	rootMenu->AddItem(menu);
	
	//Setup Rescan menu
	menu = new BMenu("Rescan");
	menu->AddItem(new BMenuItem("IDE", NULL));
	menu->AddItem(new BMenuItem("SCSI", NULL));
	rootMenu->AddItem(menu);
		
	AddChild(rootMenu);
	
}

/**
 * Handles messages.
 * @param message The message recieved by the window.
 */	
void MainWindow::MessageReceived(BMessage *message){

	BWindow::MessageReceived(message);
	
}

/**
 * Quits and Saves.
 * Sets the swap size and turns the virtual memory on by writing to the
 * /boot/home/config/settings/kernel/drivers/virtual_memory file.
 */	
bool MainWindow::QuitRequested(){

    be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
	
}

void MainWindow::FrameMoved(BPoint origin)
{//MainWindow::FrameMoved
	fSettings->SetWindowPosition(Frame());
}//MainWindow::FrameMoved

