/*
	
	Keyboard.cpp
	
*/

#ifndef KEYBOARD_WINDOW_H
#include "KeyboardWindow.h"
#endif
#ifndef KEYBOARD_H
#include "Keyboard.h"
#endif
#ifndef _ALERT_H
#include "Alert.h"
#endif
#ifndef KEYBOARD_MESSAGES_H
#include "KeyboardMessages.h"
#endif
#ifndef KEYBOARD_SETTINGS_H
#include "KeyboardSettings.h"
#endif
#include <stdio.h>
int main(int, char**)
{//main	
	KeyboardApplication	myApplication;

	myApplication.Run();

	return(0);
}//main

KeyboardApplication::KeyboardApplication():BApplication("application/x-vnd.OpenBeOS-KYBD")
{//KeyboardApplication::KeyboardApplication()
	BRect				aRect;
	
	//Create the settings object
	theseSettings = new KeyboardSettings();

	// set up a rectangle and instantiate a new window
	aRect.left=theseSettings->corner.x;
	aRect.top=theseSettings->corner.y;
	aRect.right=aRect.left+229;
	aRect.bottom=aRect.top+221;
	aWindow = new KeyboardWindow(aRect);
			
	// make window visible
	aWindow->Show();
}//KeyboardApplication::KeyboardApplication()

void KeyboardApplication::MessageReceived(BMessage *message)
{//KeyboardApplication::MessageReceived
	switch(message->what)
	{//Switch
		case ERROR_DETECTED:
			{
				aWindow->Close();
				BAlert *errorAlert = new BAlert("Error", "Something has gone wrong!","OK",NULL,NULL,B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				errorAlert->Go();
				be_app->PostMessage(B_QUIT_REQUESTED);
			}
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}//Switch
}//KeyboardApplication::MessageReceived

KeyboardApplication::~KeyboardApplication()
{//KeyboardApplication::~KeyboardApplication
		delete theseSettings;
}//KeyboardApplication::~KeyboardApplication