/*
	
	Keyboard.h

*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif

class KeyboardApplication : public BApplication 
{
public:
					KeyboardApplication();
	virtual void 	MessageReceived(BMessage *message);
};

#endif //KEYBOARD_H