/*
 * Mouse.cpp
 * Open BeOS version alpha 1 by Andrew Edward McCall mccall@digitalparadise.co.uk
 *
 */

#include <Alert.h>

#include "Mouse.h"
#include "MouseWindow.h"
#include "MouseSettings.h"
#include "MouseMessages.h"

int main(int, char**)
{
	MouseApplication	myApplication;

	myApplication.Run();

	return(0);
}

MouseApplication::MouseApplication()
					:BApplication("application/x-vnd.OpenBeOS-MOUS")
{
	fSettings = new MouseSettings();
	
	fWindow = new MouseWindow(fSettings);
			
	fWindow->Show();
}

void
MouseApplication::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case ERROR_DETECTED:
			{
				BAlert *errorAlert = new BAlert("Error", "Something has gone wrong!","OK",NULL,NULL,B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				errorAlert->Go();
				be_app->PostMessage(B_QUIT_REQUESTED);
			}
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

MouseApplication::~MouseApplication()
{
	delete fSettings;
}