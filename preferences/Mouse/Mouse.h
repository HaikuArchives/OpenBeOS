/*
	
	Mouse.h

*/

#ifndef MOUSE_H
#define MOUSE_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif

class MouseApplication : public BApplication 
{
public:
					MouseApplication();
	virtual 		~MouseApplication();
	virtual void 	MessageReceived(BMessage *message);
private:

};

#endif //MOUSE_H