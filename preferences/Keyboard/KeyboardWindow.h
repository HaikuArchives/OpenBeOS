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
#include "KeyboardSettings.h"

class KeyboardWindow : public BWindow 
{
public:
					KeyboardWindow(KeyboardSettings *fSettings);
	virtual			~KeyboardWindow();
	virtual	bool	QuitRequested();
	virtual void 	MessageReceived(BMessage *message);
	virtual void 	FrameMoved(BPoint origin);
	KeyboardView	*aView;
private:
	KeyboardSettings	*fSettings;
};

#endif //KEYBOARD_WINDOW_H
