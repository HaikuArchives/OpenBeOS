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
		if (file.Read(&fsettings, sizeof(kb_settings)) != sizeof(kb_settings))
			be_app->PostMessage(ERROR_DETECTED);
		if (file.Read(&fcorner, sizeof(BPoint)) != sizeof(BPoint))
			be_app->PostMessage(ERROR_DETECTED);
	}
	fWindowFrame.left=fcorner.x;
	fWindowFrame.top=fcorner.y;
	fWindowFrame.right=fWindowFrame.left+229;
	fWindowFrame.bottom=fWindowFrame.top+221;

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
		file.Write(&fsettings, sizeof(kb_settings));
		file.Write(&fcorner, sizeof(BPoint));
	}
}//KeyboardSettings::~KeyboardSettings

void KeyboardSettings::SetKeyboardRepeatRate(int32 f)
{//KeyboardSettings::SetKeyboardRepeatRate
	fsettings.key_repeat_rate=f;
}//KeyboardSettings::SetKeyboardRepeatRate

void KeyboardSettings::SetKeyboardDelayRate(bigtime_t f)
{//KeyboardSettings::SetKeyboardDelayRate
	fsettings.key_repeat_delay=f;
}//KeyboardSettings::SetKeyboardDelayRate

void KeyboardSettings::SetWindowPosition(BRect f)
{//KeyboardSettings::SetWindowFrame
	fcorner.x=f.left;
	fcorner.y=f.top;
}//KeyboardSettings::SetWindowFrame

