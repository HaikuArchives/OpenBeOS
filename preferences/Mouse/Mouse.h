/*
	
	Mouse.h

*/

#ifndef MOUSE_H
#define MOUSE_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef MOUSE_SETTINGS_H
#include "MouseSettings.h"
#endif
#ifndef MOUSE_WINDOW_H
#include "MouseWindow.h"
#endif

class MouseApplication : public BApplication 
{
public:
					MouseApplication();
	virtual 		~MouseApplication();
	virtual void 	MessageReceived(BMessage *message);
private:
	MouseSettings	*theseSettings;
	MouseWindow		*aWindow;

};

#endif //MOUSE_H