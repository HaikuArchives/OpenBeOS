/*
	
	Keyboard.h

*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif
#ifndef KEYBOARD_WINDOW_H
#include "KeyboardWindow.h"
#endif
#ifndef KEYBOARD_SETTINGS_H
#include "KeyboardSettings.h"
#endif

class KeyboardApplication : public BApplication 
{
public:
					KeyboardApplication();
	virtual 		~KeyboardApplication();
	virtual void 	MessageReceived(BMessage *message);
	virtual void 	AboutRequested(void);
private:
	KeyboardWindow		*aWindow;
	KeyboardSettings 	*theseSettings;
};

#endif //KEYBOARD_H