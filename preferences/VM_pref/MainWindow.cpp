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
 * @param physMem The amount of physical memory in the machine.
 * @param currSwp The current swap file size.
 * @param minVal The minimum value of the swap file.
 * @param maxSwapVal The maximum value of the swap file.
 */	
MainWindow::MainWindow(BRect frame, int physMemVal, int currSwapVal, int minVal, int maxSwapVal)
			:BWindow(frame, "Virtual Memory", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE){

	BStringView *physMem;
	BStringView *currSwap;
	BButton *defaultButton;
	BBox *topLevelView;
	BBox *boxView;
	
	/**
	 * Sets the fill color for the "used" portion of the slider.
	 */
	rgb_color fillColor;
	fillColor.red = 255;
	fillColor.blue = 0;
	fillColor.green = 0;
	
	/**
	 * This var sets the size of the visible box around the string views and
	 * the slider.
	 */
	BRect boxRect(20, 20, 240, 160);
	char labels[50];
	char sliderMinLabel[10];
	char sliderMaxLabel[10];
	
	origMemSize = currSwapVal;
	minSwapVal = minVal;
	
	/**
	 * Set up the "Physical Memory" label.
	 */
	sprintf(labels, "Physical Memory: %d MB", physMemVal);
	physMem = new BStringView(*(new BRect(10, 10, 210, 20)), "PhysicalMemory", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the "Current Swap File Size" label.
	 */
	sprintf(labels, "Current Swap File Size: %d MB", currSwapVal);
	currSwap = new BStringView(*(new BRect(10, 25, 210, 35)), "CurrentSwapSize", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the "Requested Swap File Size" label.
	 */
	sprintf(labels, "Requested Swap File Size: %d MB", currSwapVal);
	reqSwap = new BStringView(*(new BRect(10, 40, 210, 50)), "RequestedSwapSize", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the slider.
	 */
	sprintf(sliderMinLabel, "%d MB", minSwapVal);
	sprintf(sliderMaxLabel, "%d MB", maxSwapVal);
	reqSizeSlider = new BSlider(*(new BRect(10, 55, 210, 100)), "ReqSwapSizeSlider", "", new BMessage(MEMORY_SLIDER_MSG), minSwapVal, maxSwapVal, B_TRIANGLE_THUMB);
	reqSizeSlider->SetLimitLabels(sliderMinLabel, sliderMaxLabel);
	reqSizeSlider->UseFillColor(TRUE, &fillColor);
	reqSizeSlider->SetModificationMessage(new BMessage(SLIDER_UPDATE_MSG));
	
	reqSizeSlider->SetValue(currSwapVal);
	
	/**
	 * Initializes the restart notice view.
	 */
	restart = new BStringView(*(new BRect(10, 110, 210, 120)), "RestartMessage", "", B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * This view holds the three labels and the slider.
	 */
	boxView = new BBox(boxRect, "BoxView", B_FOLLOW_ALL, B_WILL_DRAW, B_FANCY_BORDER);
	boxView->AddChild(reqSizeSlider);
	boxView->AddChild(physMem);
	boxView->AddChild(currSwap);
	boxView->AddChild(reqSwap);
	boxView->AddChild(restart);
		
	defaultButton = new BButton(*(new BRect(20, 170, 80, 180)), "DefaultButton", "Default", new BMessage(DEFAULT_BUTTON_MSG), B_FOLLOW_ALL, B_WILL_DRAW);
	revertButton = new BButton(*(new BRect(100, 170, 160, 180)), "RevertButton", "Revert", new BMessage(REVERT_BUTTON_MSG), B_FOLLOW_ALL, B_WILL_DRAW);
	revertButton->SetEnabled(false);
	
	topLevelView = new BBox(Bounds(), "TopLevelView", B_FOLLOW_ALL, B_WILL_DRAW, B_NO_BORDER);
	topLevelView->AddChild(boxView);
	topLevelView->AddChild(defaultButton);
	topLevelView->AddChild(revertButton);
	
	AddChild(topLevelView);
	
}

/**
 * Handles messages.
 * @param message The message recieved by the window.
 */	
void MainWindow::MessageReceived(BMessage *message){

	char msg[100];
	
	switch(message->what){
	
		int currVal;
		
		/**
		 * Updates the requested swap file size during a drag.
		 */
		case SLIDER_UPDATE_MSG:
			
			currVal = int(reqSizeSlider->Value());
			sprintf(msg, "Requested Swap File Size: %d MB", currVal);
			reqSwap->SetText(msg);
			
			if(currVal != origMemSize){
			
				revertButton->SetEnabled(true);
				sprintf(msg, "You must reboot to apply changes.");
				restart->SetText(msg);
				
			}//if
			else{
				
				revertButton->SetEnabled(false);
				restart->SetText("");
				
			}//else
			
			break;
		
		/**
		 * Case where the slider was moved.
		 * Resets the "Requested Swap File Size" label to the new value.
		 */
		case MEMORY_SLIDER_MSG:
		
			currVal = int(reqSizeSlider->Value());
			sprintf(msg, "Requested Swap File Size: %d MB", currVal);
			reqSwap->SetText(msg);
			
			if(currVal != origMemSize){
			
				revertButton->SetEnabled(true);
				sprintf(msg, "You must reboot to apply changes.");
				restart->SetText(msg);
				
			}//if
			else{
				
				revertButton->SetEnabled(false);
				restart->SetText("");
				
			}//else
			
			break;
			
		/**
		 * Case where the default button was pressed.
		 * Eventually will set the swap file size to the optimum size, 
		 * as decided by this app (as soon as I can figure out how to 
		 * do that).
		 */
		case DEFAULT_BUTTON_MSG:
		
			reqSizeSlider->SetValue(minSwapVal);
			sprintf(msg, "Requested Swap File Size: %d MB", minSwapVal);
			reqSwap->SetText(msg);
			if(minSwapVal != origMemSize){
			
				revertButton->SetEnabled(true);
				sprintf(msg, "You must reboot to apply changes.");
				restart->SetText(msg);
				
			}//if
			else{
				
				revertButton->SetEnabled(false);
				restart->SetText("");
				
			}//else
			break;
			
		/**
		 * Case where the revert button was pressed.
		 * Returns things to the way they were when the app was started, 
		 * which is not necessarily the default size.
		 */
		case REVERT_BUTTON_MSG:
		
			revertButton->SetEnabled(false);
			sprintf(msg, "Requested Swap File Size: %d MB", origMemSize);
			reqSwap->SetText(msg);
			reqSizeSlider->SetValue(origMemSize);
			restart->SetText("");
			break;
			
		/**
		 * Unhandled messages get passed to BWindow.
		 */
		default:
		
			BWindow::MessageReceived(message);
	
	}
	
}

/**
 * Quits and Saves.
 * Sets the swap size and turns the virtual memory on by writing to the
 * /boot/home/config/settings/kernel/drivers/virtual_memory file.
 */	
bool MainWindow::QuitRequested(){

    FILE *settingsFile =
    	fopen("/boot/home/config/settings/kernel/drivers/virtual_memory", "w");
    fprintf(settingsFile, "vm on\n");
    fprintf(settingsFile, "swap_size %d\n", (int(reqSizeSlider->Value()) * 1048576));
    fclose(settingsFile);
	
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
	
}
