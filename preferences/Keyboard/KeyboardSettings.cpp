#ifndef KEYBOARD_SETTINGS_H
#include "KeyboardSettings.h"
#endif
#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef KEYBOARD_MESSAGES_H
#include "KeyboardMessages.h"
#endif
#ifndef _FILE_H
#include <File.h>
#endif
#ifndef _PATH_H
#include <Path.h>
#endif
#ifndef _FINDDIRECTORY_H
#include <FindDirectory.h>
#endif

const char KeyboardSettings::kKeyboardSettingsFile[] = "Keyboard_settings";

KeyboardSettings::KeyboardSettings()
{//KeyboardSettings::KeyboardSettings

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) == B_OK)
	{
		path.Append(kKeyboardSettingsFile);
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() != B_OK)
			be_app->PostMessage(ERROR_DETECTED);
		// Now read in the data
		file.Read(&settings, sizeof(kb_settings));
		file.Read(&corner, sizeof(BPoint));

	}

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
		file.Write(&settings, sizeof(kb_settings));
		file.Write(&corner, sizeof(BPoint));
	}
}//KeyboardSettings::~KeyboardSettings
