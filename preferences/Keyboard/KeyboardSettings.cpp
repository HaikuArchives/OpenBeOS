#ifndef KEYBOARD_SETTINGS_H
#include "KeyboardSettings.h"
#endif
#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef KEYBOARD_MESSAGES_H
#include "KeyboardMessages.h"
#endif
#include <File.h>
#include <Path.h>
#include <FindDirectory.h>
#include <stdio.h>

const char KeyboardSettings::kKeyboardSettingsFile[] = "Keyboard_settings";

KeyboardSettings::KeyboardSettings()
{//KeyboardSettings::KeyboardSettings
//write(file, &settings, sizeof (kb_settings));  // the kb_settings struct from the openbeos/input_kit kb_mouse_driver.h file
//write(file, &corner, sizeof (BPoint));  // BPoint of the location of the upper left corner of the window 
//close(file);

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) == B_OK)
	{
		path.Append(kKeyboardSettingsFile);
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() != B_OK)
			be_app->PostMessage(ERROR_DETECTED);

		file.Read(&settings, sizeof(kb_settings));
		printf("Repeat Delay = %d\n",&settings->key_repeat_delay);
		printf("Repeat Rate = %d\n",&settings->key_repeat_rate);
		file.Read(&corner, sizeof(BPoint));
		corner->PrintToStream();

	}

//	BScreen screen;
//	fWindowFrame = screen.Frame();
//	fWindowFrame.OffsetBy(-10, -10);
//	fWindowFrame.left = fWindowFrame.right - 160;
//	fWindowFrame.top = fWindowFrame.bottom - 120;


}//KeyboardSettings::KeyboardSettings

KeyboardSettings::~KeyboardSettings()
{//KeyboardSettings::~KeyboardSettings
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) < B_OK)
		return;

	path.Append(kKeyboardSettingsFile);

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	if (file.InitCheck() == B_OK)
	{
//		file.Write(&settings, sizeof(kb_settings));
//		file.Write(&corner, sizeof(BPoint));
	}
}//KeyboardSettings::~KeyboardSettings
