/*! \file MainWindow.h
    \brief Header for the MainWindow class.
    
*/

#ifndef MAIN_WINDOW_H
	
	#define MAIN_WINDOW_H
	
	#ifndef _APPLICATION_H

		#include <Application.h>
	
	#endif
	#ifndef _WINDOW_H
	
		#include <Window.h>
		
	#endif
	#ifndef _MENU_BAR_H
	
		#include <MenuBar.h>
		
	#endif
	#ifndef _MENU_ITEM_H
	
		#include <MenuItem.h>
		
	#endif
	#ifndef _RECT_H
	
		#include <Rect.h>
		
	#endif
	#ifndef _STDIO_H
	
		#include <stdio.h>
		
	#endif
	#ifndef POS_SETTINGS_H
	
		#include "PosSettings.h"

	#endif
	
	/**
	 * The main window of the app.
	 *
	 * Sets up and displays everything you need for the app.
	 */
	class MainWindow : public BWindow{
	
		private:
		
			PosSettings	*fSettings;
			BMenuBar *rootMenu;
			
		public:
		
			MainWindow(BRect frame, PosSettings *fSettings);
			virtual bool QuitRequested();
			virtual void MessageReceived(BMessage *message);
			virtual void FrameMoved(BPoint origin);
						
	};
	
#endif
