// Media Server Definitions
#ifndef MEDIA_SERVER_DEFS
#define MEDIA_SERVER_DEFS


#include <SupportDefs.h> //B_OK and that kind of stuff.
#include <Message.h>
#include <Application.h>
#include <List.h>
#include "ServerClientNotifications.h"
#include "ClientServerModel.h"
#include "MediaWindow.h"
#include "MediaServerTypes.h"





//Definition of the Media Server class. ======================================= 
class MediaServerApp_Class		:public BApplication, public Client_Class
	{
	private:
	bool 	media_server_has_started;
	BList 	*RegisteredApplications;
	BList 	*FoundDevices;
	int 	used_device_id;
		
	public:
	//=============
	//Media Server Events you can watch with BMediaRoster
	Server_Class *AppRegistered;
	Server_Class *AppUnregistered;
	Server_Class *MediaServerStarted;
	Server_Class *MediaServerDying;
	Server_Class *NewNodeAdded;   		//B_MEDIA_NODE_CREATED
	Server_Class *OldNodeDeleted; 		//B_MEDIA_NODE_DELETED
	Server_Class *Connected; 			//B_MEDIA_CONNECTION_MADE
	Server_Class *Disconnected;			//B_MEDIA_CONNECTION_BROKEN
	Server_Class *BufferCreated;		//B_MEDIA_BUFFER_CREATED
	Server_Class *BufferDeleted; 		//B_MEDIA_BUFFER_DELETED
	Server_Class *TransportState;		//B_MEDIA_TRANSPORT_STATE
	Server_Class *ParameterChanged;		//B_MEDIA_PARAMETER_CHANGED
	Server_Class *FormatChanged;		//B_MEDIA_FORMAT_CHANGED
	Server_Class *MediaWebChanged;		//B_MEDIA_WEB_CHANGED
	Server_Class *MediaDefaultChanged;	//B_MEDIA_DEFAULT_CHANGED
	Server_Class *NewParameter;			//B_MEDIA_NEW_PARAMETER
	Server_Class *NodeStopped;			//B_MEDIA_NODE_STOPPED
	Server_Class *MediaFlavorsChanged;	//B_MEDIA_FLAVORS_CHANGED
	Server_Class *DevicesChanged;		//Notifies when there is a change in the /dev/audio device drivers - not implemented
	//=============
	MediaServerApp_Class			( void );
	virtual ~MediaServerApp_Class	( void );
	
	virtual void ReadyToRun 		( void );
	virtual bool QuitRequested		( void );
	virtual void MessageReceived	( BMessage *message_to_media_server );
	
	status_t InitializeHardware		( void );
	status_t FindHardware			( void );
	int 	 DeviceCount			( void );
	status_t UseDevice				( int device_number ); //if there is more than one device you can specify which one to use
	status_t StopServer				( void );
	status_t LoadSettingsFromDisk	( void );
	
	status_t RegisterMediaApplication	( BMessage *registration_message );
	status_t UnRegisterMediaApplication	( BMessage *registration_message );
	
	//from Client_Class
	virtual void OnClientNotified( void * , int ID);
	};


//Defined in "MediaServer.cpp"
extern MediaServerApp_Class		*media_server;
extern MediaServerExit_Type 	media_server_exit_code; 

#endif