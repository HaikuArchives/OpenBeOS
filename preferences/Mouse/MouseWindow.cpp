/*
	
	MouseWindow.cpp
	
*/

#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef MOUSE_WINDOW_H
#include "MouseWindow.h"
#endif
#ifndef MOUSE_MESSAGES_H
#include "MouseMessages.h"
#endif
#ifndef _BUTTON_H
#include <Button.h>
#endif
#include <stdio.h>
#include <Message.h>
MouseWindow::MouseWindow(MouseSettings *Settings)
				: BWindow(Settings->WindowPosition(), "Mouse", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{//MouseWindow::MouseWindow
	// set up a rectangle and instantiate a new view
	fSettings = Settings;
	BRect aRect( Bounds() );
	aView = new MouseView(aRect);
	// add view to window
	AddChild(aView);
}//MouseWindow::MouseWindow

bool MouseWindow::QuitRequested()
{//MouseWindow::QuitRequested
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}//MouseWindow::QuitRequested

void MouseWindow::MessageReceived(BMessage *message)
{//MouseWindow::MessageReceived
	switch(message->what)
	{//Switch
		case BUTTON_DEFAULTS:
			{//BUTTON_DEFAULTS
			}//BUTTON_DEFAULTS
			break;
		case BUTTON_REVERT:
			{//BUTTON_REVERT
				//set_mouse_speed();
			}//BUTTON_REVERT
			break;
		case SLIDER_DOUBLE_CLICK_SPEED:
			{//SLIDER_DOUBLE_CLICK_SPEED
				// We need to minus the value in the BMessage because the
				// slide is 0 to 1000000 rather than 1000000 to 0.
				set_click_speed(1000000-message->FindInt32("be:value"));
			}//SLIDER_DOUBLE_CLICK_SPEED
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}//Switch
}//MouseWindow::MessageReceived

MouseWindow::~MouseWindow()
{//

}//

void MouseWindow::FrameMoved(BPoint origin)
{//MouseWindow::FrameMoved
	fSettings->SetWindowPosition(Frame());
}//MouseWindow::FrameMoved