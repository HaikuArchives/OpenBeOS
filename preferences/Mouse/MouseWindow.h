#ifndef MOUSE_WINDOW_H
#define MOUSE_WINDOW_H

#include <Window.h>

#include "MouseView.h"
#include "MouseSettings.h"

class MouseWindow : public BWindow 
{
public:
	MouseWindow(MouseSettings *settings);
	virtual	~MouseWindow();
	
	virtual	bool QuitRequested();
	virtual void MessageReceived(BMessage *message);
	virtual void FrameMoved(BPoint origin);
	
private:
	MouseSettings	*fSettings;
	MouseView		*fView;
};

#endif
