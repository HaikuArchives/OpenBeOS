/*! \file MainWindow.h
    \brief Header for the MainWindow class.
    
*/

#ifndef MAIN_WINDOW_H

	
	#define MAIN_WINDOW_H
	
	/*!
	 *  Default button message.
	 */
	#define DEFAULT_BUTTON_MSG 'dflt'
	
	/*!
	 *  Revert button message.
	 */
	#define REVERT_BUTTON_MSG 'rvrt'
	
	/*!
	 *  Slider message.
	 */
	#define MEMORY_SLIDER_MSG 'sldr'
	
	#ifndef _APPLICATION_H

		#include <Application.h>
	
	#endif
	#ifndef _WINDOW_H
	
		#include <Window.h>
		
	#endif
	#ifndef _STRING_VIEW_H
	
		#include <StringView.h>
		
	#endif
	#ifndef _BOX_H
	
		#include <Box.h>
		
	#endif
	#ifndef _SLIDER_H
	
		#include <Slider.h>
		
	#endif
	#ifndef _BUTTON_H
	
		#include <Button.h>
		
	#endif
	#ifndef _STDIO_H
	
		#include <stdio.h>
		
	#endif
	
	/**
	 * The main window of the app.
	 *
	 * Sets up and displays everything you need for the app.
	 */
	class MainWindow : public BWindow{
	
		private:
		
			/**
			 * Saves the size of the swap file when the app is started
			 * so that it can be restored later if need be.
			 */
			int origMemSize;
			
			/**
			 * The BStringView that shows the requested swap file 
			 * size.
			 */
			BStringView *reqSwap;
			
			/**
			 * The slider that lets you adjust the size of the swap file.
			 */
			BSlider *reqSizeSlider;
			
			BButton *revertButton;
			
		public:
		
			/**
			 * Constructor.
			 * @param frame The size to make the window.
			 * @param physMem The amount of physical memory in the machine.
			 * @param currSwp The current swap file size.
			 * @param sliderMin The minimum value of the swap file.
			 * @param sliderMax The maximum value of the swap file.
			 */			 
			MainWindow(BRect frame, int physMem, int currSwp, int sliderMin, int sliderMax);
			
			/**
			 * Handles the quit message.
			 * \attention This is where the code to save the swap 
			 * file size should go!
			 */
			virtual bool QuitRequested();
			
			/**
			 * Handles application messages.
			 */
			virtual void MessageReceived(BMessage *message);
						
	};
	
#endif
