#ifndef CONNECT_APP_H
#define CONNECT_APP_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include "View.h"
#include "MsgConsts.h"

#define APPLICATION "Application"
#define VERSION "1.0"

class AppWindow : public BWindow {
public:
	BMenuBar *menubar;
	View *view;
	AppWindow(BRect);
	bool QuitRequested();
	void AboutRequested();	
	void MessageReceived(BMessage *message);
};

class App : public BApplication {
public:
	AppWindow *window;
	App();
};

#define my_app ((App*)be_app)
#endif