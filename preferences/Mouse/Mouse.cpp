/*
	
	Keyboard.cpp
	
*/

#ifndef KEYBOARD_H
#include "Mouse.h"
#endif
#ifndef _ALERT_H
#include <Alert.h>
#endif
#ifndef MOUSE_MESSAGES_H
#include "MouseMessages.h"
#endif

int main(int, char**)
{//main	
	MouseApplication	myApplication;

	myApplication.Run();

	return(0);
}//main

MouseApplication::MouseApplication():BApplication("application/x-vnd.OpenBeOS-MOUS")
{//MouseApplication::MouseApplication()
	;
}//MouseApplication::MouseApplication()

void MouseApplication::MessageReceived(BMessage *message)
{//MouseApplication::MessageReceived
	switch(message->what)
	{//Switch
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
	}//Switch
}//MouseApplication::MessageReceived

MouseApplication::~MouseApplication()
{//MouseApplication::~MouseApplication
	;
}//MouseApplication::~MouseApplication