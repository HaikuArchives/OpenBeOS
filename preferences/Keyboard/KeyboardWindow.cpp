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
#include <Beep.h>
#include <Alert.h>
KeyboardWindow::KeyboardWindow(BRect frame)
				: BWindow(frame, "Keyboard", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{//KeyboardWindow::KeyboardWindow
	KeyboardView	*aView;
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
			{
				;
			}
			break;
		case BUTTON_REVERT:
			{
				;
			}
			break;
		case SLIDER_REPEAT_RATE:
			{
				    //rrate = rslider->Value();
					//if (set_key_repeat_rate(rrate)!=B_OK) 
	  					//{
	    					//be_app->PostMessage(B_QUIT_REQUESTED);
	  					//};
			}
			break;
		case SLIDER_DELAY_RATE:
			{
				;
			}
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}//Switch
}//KeyboardWindow::MessageReceived