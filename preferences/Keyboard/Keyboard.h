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

class KeyboardApplication : public BApplication 
{
public:
					KeyboardApplication();
	virtual void 	MessageReceived(BMessage *message);
private:
	KeyboardWindow		*aWindow;
};

#endif //KEYBOARD_H