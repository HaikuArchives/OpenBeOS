/*
	
	KeyboardWindow.h
	
*/

#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#ifndef _WINDOW_H
#include <Window.h>
#endif
#ifndef KEYBOARD_VIEW_H
#include "KeyboardView.h"
#endif

class KeyboardWindow : public BWindow 
{
public:
					KeyboardWindow(BRect frame); 
	virtual	bool	QuitRequested();
	virtual void 	MessageReceived(BMessage *message);
	KeyboardView	*aView;
};

#endif //KEYBOARD_WINDOW_H
