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
#ifndef KEYBOARD_SETTINGS_H
#include "KeyboardSettings.h"
#endif

class KeyboardWindow : public BWindow 
{
public:
					KeyboardWindow(KeyboardSettings *fSettings);
	virtual			~KeyboardWindow();
	virtual	bool	QuitRequested();
	virtual void 	MessageReceived(BMessage *message);
	virtual void 	FrameMoved(BPoint origin);
private:
	KeyboardSettings	*fSettings;
	KeyboardView		*aView;
};

#endif //KEYBOARD_WINDOW_H
