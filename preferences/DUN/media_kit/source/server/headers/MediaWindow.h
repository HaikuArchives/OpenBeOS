#ifndef MEDIASERVERWINDOW
#define MEDIASERVERWINDOW

#include <InterfaceKit.h>
#include <Message.h>
#include "ClientServerModel.h"
#include "ServerClientNotifications.h"

class MediaWindow_Class : public BWindow, public Client_Class
	{
	BView *Background;
	BButton *QuitButton;
	BListView *RegisteredTeams;
	BListView *RegisteredNodes;
	BListView *DeviceList;
	
	
	public:
	
	Server_Class *WindowIsDying;
	Server_Class *ServerDeathRequest;
	Server_Class *NewDeviceSelection;
	
	MediaWindow_Class();
	~MediaWindow_Class();
	
	virtual void MessageReceived( BMessage *window_message );
	virtual void OnClientNotified( void * , int ID);
	};

#endif