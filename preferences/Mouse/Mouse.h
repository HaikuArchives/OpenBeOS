#ifndef MOUSE_H
#define MOUSE_H

#include <Application.h>

#include "MouseWindow.h"
#include "MouseSettings.h"

class MouseApplication : public BApplication 
{
public:
	MouseApplication();
	virtual ~MouseApplication();
	
	virtual void MessageReceived(BMessage *message);
	
private:

	MouseSettings	*fSettings;
	MouseWindow		*fWindow;
};

#endif