/*! \file MainWindow.cpp
    \brief Code for the MainWindow class.
    
    Displays the main window, the essence of the app.
*/

#ifndef MAIN_WINDOW_H

	#include "MainWindow.h"
	
#endif

/**
 * Constructor.
 * @param frame The size to make the window.
 * @param physMem The amount of physical memory in the machine.
 * @param currSwp The current swap file size.
 * @param sliderMin The minimum value of the swap file.
 * @param sliderMax The maximum value of the swap file.
 */	
MainWindow::MainWindow(BRect frame, int physMemVal, int currSwapVal, int minSwapVal, int maxSwapVal)
			:BWindow(frame, "Virtual Memory", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE){

	BStringView *physMem;
	BStringView *currSwap;
	BButton *defaultButton;
	BBox *topLevelView;
	BBox *boxView;
	/**
	 * This var sets the size of the visible box around the string views and
	 * the slider.
	 */
	BRect boxRect(20, 20, 240, 140);
	char labels[50];
	char sliderMinLabel[10];
	char sliderMaxLabel[10];
	
	origMemSize = currSwapVal;
	
	/**
	 * Set up the "Physical Memory" label.
	 */
	sprintf(labels, "Physical Memory: %d", physMemVal);
	physMem = new BStringView(*(new BRect(10, 10, 210, 20)), "PhysicalMemory", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the "Current Swap File Size" label.
	 */
	sprintf(labels, "Current Swap File Size: %d", currSwapVal);
	currSwap = new BStringView(*(new BRect(10, 25, 210, 35)), "CurrentSwapSize", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the "Requested Swap File Size" label.
	 */
	sprintf(labels, "Requested Swap File Size: %d", minSwapVal);
	reqSwap = new BStringView(*(new BRect(10, 40, 210, 50)), "RequestedSwapSize", labels, B_FOLLOW_ALL, B_WILL_DRAW);
	
	/**
	 * Set up the slider.
	 */
	sprintf(sliderMinLabel, "%d MB", minSwapVal);
	sprintf(sliderMaxLabel, "%d MB", maxSwapVal);
	reqSizeSlider = new BSlider(*(new BRect(10, 55, 210, 140)), "ReqSwapSizeSlider", "", new BMessage(MEMORY_SLIDER_MSG), minSwapVal, maxSwapVal, B_TRIANGLE_THUMB);
	reqSizeSlider->SetLimitLabels(sliderMinLabel, sliderMaxLabel);
	
	/**
	 * This view holds the three labels and the slider.
	 */
	boxView = new BBox(boxRect, "BoxView", B_FOLLOW_ALL, B_WILL_DRAW, B_FANCY_BORDER);
	boxView->AddChild(reqSizeSlider);
	boxView->AddChild(physMem);
	boxView->AddChild(currSwap);
	boxView->AddChild(reqSwap);
		
	defaultButton = new BButton(*(new BRect(20, 160, 80, 180)), "DefaultButton", "Default", new BMessage(DEFAULT_BUTTON_MSG), B_FOLLOW_ALL, B_WILL_DRAW);
	revertButton = new BButton(*(new BRect(100, 160, 160, 180)), "RevertButton", "Revert", new BMessage(REVERT_BUTTON_MSG), B_FOLLOW_ALL, B_WILL_DRAW);
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
	
		/**
		 * Case where the slider was moved.
		 * Resets the "Requested Swap File Size" label to the new value.
		 */
		case MEMORY_SLIDER_MSG:
		
			sprintf(msg, "Requested Swap File Size: %d", int(reqSizeSlider->Value()));
			reqSwap->SetText(msg);
			revertButton->SetEnabled(true);
			break;
			
		/**
		 * Case where the default button was pressed.
		 * Eventually will set the swap file size to the optimum size, 
		 * as decided by this app (as soon as I can figure out how to 
		 * do that).
		 */
		case DEFAULT_BUTTON_MSG:
		
			break;
			
		/**
		 * Case where the revert button was pressed.
		 * Returns things to the way they were when the app was started, 
		 * which is not necessarily the default size.
		 */
		case REVERT_BUTTON_MSG:
		
			revertButton->SetEnabled(false);
			sprintf(msg, "Requested Swap File Size: %d", origMemSize);
			reqSwap->SetText(msg);
			reqSizeSlider->SetValue(origMemSize);
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
 * \attention This is where the code to save the swap 
 * file size should go!
 */	
bool MainWindow::QuitRequested(){

	/*
	 * put in code to save the value here
	 */
	
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
	
}
