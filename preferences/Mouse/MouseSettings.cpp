#ifndef MOUSE_SETTINGS_H
#include "MouseSettings.h"
#endif
#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef MOUSE_MESSAGES_H
#include "MouseMessages.h"
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

#include <stdio.h>

const char MouseSettings::kMouseSettingsFile[] = "Mouse_settings";

MouseSettings::MouseSettings()
{//MouseSettings::MouseSettings

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) == B_OK)
	{
		path.Append(kMouseSettingsFile);
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() != B_OK)
			be_app->PostMessage(ERROR_DETECTED);
		// Now read in the data
		if (file.Read(&fsettings, sizeof(mouse_settings)) != sizeof(mouse_settings))
			be_app->PostMessage(ERROR_DETECTED);
		if (file.Read(&fcorner, sizeof(BPoint)) != sizeof(BPoint))
			be_app->PostMessage(ERROR_DETECTED);
	}
	printf("Mouse settings file read.\n");
	printf("=========================\n");
	printf("fsettings.type is %d\n",(int)fsettings.type);
	printf("fsettings.map.left is %d\n",(int)fsettings.map.left);
	printf("fsettings.map.middle is %d\n",(int)fsettings.map.middle);
	printf("fsettings.map.right is %d\n",(int)fsettings.map.right);
	printf("fsettings.accel.enabled is ");
	if (fsettings.accel.enabled) {printf ("true\n");} else {printf ("false\n");}
	printf("fsettings.accel.accel_factor is %ld\n",fsettings.accel.accel_factor);
	printf("fsettings.accel.speed is %ld\n",fsettings.accel.speed);
	printf("fsettings.click_speed is %ld\n",(long)fsettings.click_speed);
	printf("fcorner read in as ");
	fcorner.PrintToStream();

	fWindowFrame.left=fcorner.x;
	fWindowFrame.top=fcorner.y;
	fWindowFrame.right=fWindowFrame.left+397;
	fWindowFrame.bottom=fWindowFrame.top+293;

}//MouseSettings::MouseSettings

MouseSettings::~MouseSettings()
{//MouseSettings::~MouseSettings
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) < B_OK)
		return;

	path.Append(kMouseSettingsFile);

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	if (file.InitCheck() == B_OK)
	{
		file.Write(&fsettings, sizeof(mouse_settings));
		file.Write(&fcorner, sizeof(BPoint));
	}
}//MouseSettings::~MouseSettings

void MouseSettings::SetWindowPosition(BRect f)
{//MouseSettings::SetWindowFrame
	fcorner.x=f.left;
	fcorner.y=f.top;
}//MouseSettings::SetWindowFrame