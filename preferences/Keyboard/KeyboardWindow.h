/*
	
	KeyboardWindow.h
	
*/

#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#ifndef _WINDOW_H
#include <Window.h>
#endif

class KeyboardWindow : public BWindow 
{
public:
					KeyboardWindow(BRect frame); 
	virtual	bool	QuitRequested();
	virtual void 	MessageReceived(BMessage *message);
};

#endif //KEYBOARD_WINDOW_H
