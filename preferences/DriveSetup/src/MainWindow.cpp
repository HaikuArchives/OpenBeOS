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
	menu->AddItem(new BMenuItem("Mount All Partitions", new BMessage(MOUNT_MOUNT_ALL_MSG), 'M'));
	//add menu items for list of partitions
	rootMenu->AddItem(menu);
	
	//Setup Unmount menu
	menu = new BMenu("Unmount");
	//add menu items for list of partitions
	rootMenu->AddItem(menu);
	
	//Setup Setup menu
	menu = new BMenu("Setup");
	menu->AddItem(new BMenuItem("Format", new BMessage(SETUP_FORMAT_MSG), 'F'));
	tmpMenu = new BMenu("Partition");
	tmpMenu->AddItem(new BMenuItem("apple...", new BMessage(SETUP_PARTITION_APPLE_MSG)));
	tmpMenu->AddItem(new BMenuItem("intel...", new BMessage(SETUP_PARTITION_INTEL_MSG)));
	menu->AddItem(tmpMenu);
	tmpMenu = new BMenu("Initialize");
	//add menu items for list of partitions
	menu->AddItem(tmpMenu);
	
	rootMenu->AddItem(menu);
	
	//Setup Options menu
	menu = new BMenu("Options");
	menu->AddItem(new BMenuItem("Eject", new BMessage(OPTIONS_EJECT_MSG), 'E'));
	menu->AddItem(new BMenuItem("Surface Test", new BMessage(OPTIONS_SURFACE_TEST_MSG), 'T'));
	rootMenu->AddItem(menu);
	
	//Setup Rescan menu
	menu = new BMenu("Rescan");
	menu->AddItem(new BMenuItem("IDE", new BMessage(RESCAN_IDE_MSG)));
	menu->AddItem(new BMenuItem("SCSI", new BMessage(RESCAN_SCSI_MSG)));
	rootMenu->AddItem(menu);
		
	AddChild(rootMenu);
	
}

/**
 * Handles messages.
 * @param message The message recieved by the window.
 */	
void MainWindow::MessageReceived(BMessage *message){

	switch(message->what){
	
		case MOUNT_MOUNT_ALL_MSG:
		
			printf("MOUNT_MOUNT_ALL_MSG\n");
			break;
			
		case MOUNT_MOUNT_SELECTED_MSG:
		
			printf("MOUNT_MOUNT_SELECTED_MSG\n");
			break;
			
		case UNMOUNT_UNMOUNT_SELECTED_MSG:
		
			printf("UNMOUNT_UNMOUNT_SELECTED_MSG\n");
			break;
		
		case SETUP_FORMAT_MSG:
		
			printf("SETUP_FORMAT_MSG\n");
			break;
			
		case SETUP_PARTITION_APPLE_MSG:
		
			printf("SETUP_PARTITION_APPLE_MSG\n");
			break;
			
		case SETUP_PARTITION_INTEL_MSG:
		
			printf("SETUP_PARTITION_INTEL_MSG\n");
			break;
			
		case SETUP_INITIALIZE_MSG:
		
			printf("SETUP_INITIALIZE_MSG\n");
			break;
			
		case OPTIONS_EJECT_MSG:
		
			printf("OPTIONS_EJECT_MSG\n");
			break;
			
		case OPTIONS_SURFACE_TEST_MSG:
		
			printf("OPTIONS_SURFACE_TEST_MSG\n");
			break;
			
		case RESCAN_IDE_MSG:
		
			printf("RESCAN_IDE_MSG\n");
			break;
			
		case RESCAN_SCSI_MSG:
		
			printf("RESCAN_SCSI_MSG\n");
			break;
			
		default:
		
			BWindow::MessageReceived(message);
			
	}//switch
	
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

void MainWindow::AddDeviceInfo(dev_info d){

	printf("Got %s\n", d.device);
	//refresh the devices view

}//SetDeviceInfo