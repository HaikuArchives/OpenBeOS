/*
	
	KeyboardWindow.cpp
	
*/

#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef KEYBOARD_WINDOW_H
#include "KeyboardWindow.h"
#endif
#ifndef KEYBOARD_VIEW_H
#include "KeyboardView.h"
#endif
#ifndef KEYBOARD_MESSAGES_H
#include "KeyboardMessages.h"
#endif

KeyboardWindow::KeyboardWindow(BRect frame)
				: BWindow(frame, "Keyboard", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{//KeyboardWindow::KeyboardWindow
	// set up a rectangle and instantiate a new view
	BRect aRect( Bounds() );
	aView = new KeyboardView(aRect);
	// add view to window
	AddChild(aView);
}//KeyboardWindow::KeyboardWindow

bool KeyboardWindow::QuitRequested()
{//KeyboardWindow::QuitRequested
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}//KeyboardWindow::QuitRequested

void KeyboardWindow::MessageReceived(BMessage *message)
{//KeyboardWindow::MessageReceived
	switch(message->what)
	{//Switch
		case BUTTON_DEFAULTS:
			{//BUTTON_DEFAULTS
				if (set_key_repeat_rate(200)!=B_OK) 
	  			{
	    			be_app->PostMessage(ERROR_DETECTED);
	  			};
	  			aView->rateSlider->SetValue(200);
				if (set_key_repeat_delay(250000)!=B_OK) 
	  			{
	    			be_app->PostMessage(ERROR_DETECTED);
	  			};
	  			aView->delaySlider->SetValue(250000);
			}//BUTTON_DEFAULTS
			break;
		case BUTTON_REVERT:
			{//BUTTON_REVERT
				;
			}//BUTTON_REVERT
			break;
		case SLIDER_REPEAT_RATE:
			{//SLIDER_REPEAT_RATE
				if (set_key_repeat_rate(aView->rateSlider->Value())!=B_OK) 
	  			{
	    			be_app->PostMessage(ERROR_DETECTED);
	  			};
			}//SLIDER_REPEAT_RATE
			break;
		case SLIDER_DELAY_RATE:
			{//SLIDER_DELAY_RATE
				if (set_key_repeat_delay(aView->delaySlider->Value())!=B_OK) 
	  			{
	    			be_app->PostMessage(ERROR_DETECTED);
	  			};;
			}//SLIDER_DELAY_RATE
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}//Switch
}//KeyboardWindow::MessageReceived