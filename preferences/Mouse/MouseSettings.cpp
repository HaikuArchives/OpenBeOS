/*
 * MouseSettings.cpp
 * Open BeOS version alpha 1 by Andrew Edward McCall mccall@digitalparadise.co.uk
 *
 * Thanks go to François Revol revol@free.fr
 */
 
#include <Application.h>
#include <FindDirectory.h>
#include <File.h>
#include <Path.h>

#include "MouseSettings.h"
#include "MouseMessages.h"

#include <stdio.h>

const char MouseSettings::kMouseSettingsFile[] = "Mouse_settings";

MouseSettings::MouseSettings()
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) == B_OK) {
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
	
	//Check to see if the co-ords of the window are in the range of the Screen
	BScreen screen;
		if (screen.Frame().right >= fWindowFrame.right
			&& screen.Frame().bottom >= fWindowFrame.bottom)
		return;
	// If they are not, lets just stick the window in the middle
	// of the screen.
	fWindowFrame = screen.Frame();
	fWindowFrame.left = (fWindowFrame.right-397)/2;
	fWindowFrame.right = fWindowFrame.left + 397;
	fWindowFrame.top = (fWindowFrame.bottom-293)/2;
	fWindowFrame.bottom = fWindowFrame.top + 293;

}

MouseSettings::~MouseSettings()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path) < B_OK)
		return;

	path.Append(kMouseSettingsFile);

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	if (file.InitCheck() == B_OK) {
		file.Write(&fsettings, sizeof(mouse_settings));
		file.Write(&fcorner, sizeof(BPoint));
	}
}

void
MouseSettings::SetWindowPosition(BRect f)
{
	fcorner.x=f.left;
	fcorner.y=f.top;
}