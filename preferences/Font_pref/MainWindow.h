#ifndef MAIN_WINDOW_H

	#define MAIN_WINDOW_H

	#ifndef _APPLICATION_H
	
		#include <Application.h>
	
	#endif
	#ifndef _WINDOW_H
			
		#include <Window.h>
	
	#endif
	#ifndef _TAB_VIEW_H
		
		#include <TabView.h>
		
	#endif

	#ifndef FONT_VIEW_H
	
		#include "FontView.h"
		
	#endif
	
	#ifndef BUTTON_VIEW_H
	
		#include "ButtonView.h"
		
	#endif
		
	#ifndef _BOX_H
	
		#include <Box.h>
		
	#endif
	
	class MainWindow : public BWindow{
	
		public:
			
			MainWindow(BRect frame); 
			virtual	bool QuitRequested();
			virtual void MessageReceived(BMessage *message);
			
		private:
		
			FontView *fontPanel;
			void updateSize(FontSelectionView *theView);
			void updateFont(FontSelectionView *theView);
			void updateStyle(FontSelectionView *theView);
			ButtonView *buttonView;
	
	};

#endif

