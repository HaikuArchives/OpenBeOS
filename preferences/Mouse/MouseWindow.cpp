/*
 * MouseWindow.cpp
 * Open BeOS version alpha 1 by Andrew Edward McCall mccall@digitalparadise.co.uk
 *
 */
 
#include <Application.h>
#include <Button.h>
#include <Message.h>

#include "MouseWindow.h"
#include "MouseMessages.h"


MouseWindow::MouseWindow(MouseSettings *settings)
				: BWindow(settings->WindowPosition(), "Mouse", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	fSettings = settings;
	BRect aRect( Bounds() );
	fView = new MouseView(aRect);
	AddChild(fView);
}

bool MouseWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

void MouseWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case BUTTON_DEFAULTS:
			{
			}
			break;
		case BUTTON_REVERT:
			{
				//set_mouse_speed();
			}
			break;
		case SLIDER_DOUBLE_CLICK_SPEED:
			{
				// We need to minus the value in the BMessage because the
				// slide is 0 to 1000000 rather than 1000000 to 0.
				set_click_speed(1000000-message->FindInt32("be:value"));
			}
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

MouseWindow::~MouseWindow()
{

}

void MouseWindow::FrameMoved(BPoint origin)
{
	fSettings->SetWindowPosition(Frame());
}