/*
	
	MouseWindow.h
	
*/

#ifndef MOUSE_WINDOW_H
#define MOUSE_WINDOW_H

#ifndef _WINDOW_H
#include <Window.h>
#endif
#ifndef MOUSE_SETTINGS_H
#include "MouseSettings.h"
#endif
#ifndef MOUSE_VIEW_H
#include "MouseView.h"
#endif

class MouseWindow : public BWindow 
{
public:
					MouseWindow(MouseSettings *fSettings);
	virtual			~MouseWindow();
	virtual	bool	QuitRequested();
	virtual void 	MessageReceived(BMessage *message);
	virtual void 	FrameMoved(BPoint origin);
private:
	MouseSettings	*fSettings;
	MouseView		*aView;
};

#endif //MOUSE_WINDOW_H
