






/*###############################################################################*\
  #                                                                             #
  #        Open BeOS Server                                                     #
  #        New Media Server                                                     #
  #                                                                             #
  #        Level : Development                                                  #
  #                                                                             #
  #        Media Kit Team Leader : Marcus Overhagen                             #
  #        Author                : Aleksander Lodwich                           #
  #                                                                             #
  #                                                                             #
  #        MODULE: Main Module Nov 2001                                         #
\*###############################################################################*/


/*# Additional Comments #############################################################*\
  #                                                                                 #
  # - Note there is a 999 Bytes limit for each line in the setting file.            #
  #                                                                                 #
  # - The only setting that can be stored yet is the master volume value.           #
  #   The settings file line must look something like this                          #
  #   "MasterVolume:0.274"                                                          #
  #   "MasterVolume:1.000"                                                          #
  #   Spaces are not accepted! It must be a 4 digit number with a dot in second!    #
  #   valid Value range: 0.000 - 9.999                                              #  
  #                                                                                 #  
  #                                                                                 #  
  #  The server avoids using the libmedia.so yet. Thus you will find no API classes #
  #  used in this program. I consider it enough when the classes in here will       #
  #  behave as their real counterparts.                                             #
  #                                                                                 #  
  #  This concerns:                                                                 #      
  #     MixerNode_Class    //System Mixer Device                                    #
  #     SystemTime_Class   //System Time Source                                     #
  #     Buffer_Class       //special Media Server Buffers for BPlaySound MSG.       #
\*###################################################################################*/








#include <Application.h>
#include <Window.h>
#include <Looper.h>
#include <Message.h>
#include <MediaDefs.h>
#include <OS.h>
#include <FindDirectory.h>
#include <Path.h>
#include <File.h>
#include <Roster.h>
#include "MediaServer.h"
#include "PrivateFunctionsModule.h"
#include "AudioDeviceDetection.h"



MediaServerExit_Type 	last_media_server_status;
MediaServerApp_Class	*media_server;
MediaWindow_Class		*global_window=0;











//================================================================================
//================================================================================
//Implementation of the MediaServerApp_Class======================================
/*======*/
/*======*/
MediaServerApp_Class::MediaServerApp_Class( void )
:BApplication( NEW_MEDIA_SERVER_SIGNATURE )
{
media_server_has_started 	= false;
RegisteredApplications 		= new BList;
FoundDevices				= new BList;

AppRegistered 		= new Server_Class( EVENT_APP_REGISTERED 	);
AppUnregistered		= new Server_Class( EVENT_APP_UNREGISTERED 	);
MediaServerStarted	= new Server_Class( EVENT_SERVER_STARTED 	);
MediaServerDying	= new Server_Class( EVENT_SERVER_DYING 		);
NewNodeAdded		= new Server_Class( EVENT_NODE_CREATED 		);
OldNodeDeleted		= new Server_Class( EVENT_NODE_DELETED 		);
Connected			= new Server_Class( EVENT_CONNECTION_MADE	);
Disconnected		= new Server_Class( EVENT_CONNECTION_BROKEN	);
BufferCreated		= new Server_Class( EVENT_BUFFER_CREATED	);
BufferDeleted		= new Server_Class( EVENT_BUFFER_DELETED	);
TransportState		= new Server_Class( EVENT_TRANSPORT_STATE	);
ParameterChanged	= new Server_Class( EVENT_PARAMETER_CHANGED	);
FormatChanged		= new Server_Class( EVENT_FORMAT_CHANGED	);
MediaWebChanged		= new Server_Class( EVENT_WEB_CHANGED		);
MediaDefaultChanged	= new Server_Class( EVENT_DEFAULT_CHANGED	);
NewParameter		= new Server_Class( EVENT_NEW_PARAMETER		);
NodeStopped			= new Server_Class( EVENT_NODE_STOPPED		);
MediaFlavorsChanged	= new Server_Class( EVENT_FLAVORS_CHANGED	);
DevicesChanged		= new Server_Class( EVENT_UPDATE_DEVICE_LIST);
}



/*======*/
/*======*/
MediaServerApp_Class::~MediaServerApp_Class( void )
{
MediaServerDying->NotifyClients();

if (media_server_has_started == true)
	{
	BString *any;
	int i,c;
	//clean up things that can only have happened 
	//when the media server class was used.
	c = FoundDevices->CountItems();
	for (i=0; i<c; ++i)
		{
		any = (BString *) FoundDevices->ItemAt(i);
		delete any;
		}
	}
delete RegisteredApplications;
delete FoundDevices;

delete AppRegistered;
delete AppUnregistered;
delete MediaServerStarted;
delete MediaServerDying;
delete NewNodeAdded;
delete OldNodeDeleted;
delete Connected;
delete Disconnected;
delete BufferCreated;
delete BufferDeleted;
delete TransportState;
delete ParameterChanged;
delete FormatChanged;
delete MediaWebChanged;
delete MediaDefaultChanged;
delete NewParameter;
delete NodeStopped;
delete MediaFlavorsChanged;
delete DevicesChanged;
}



/*======*/
/*======*/
void MediaServerApp_Class::ReadyToRun( void )
{
media_server_has_started = true;

thread_id 	media_server_thread = Thread();
set_thread_priority( media_server_thread, MEDIA_SERVER_PRIORITY );

//the following cannot be yet implemented because it would require libmedia.so to be linked to this, 
//but we don´t want to do this. The locking must be done somehow with the Kernel Kit.

//lock the media server image in memory
//status_t 	result_of_locking_server = media_realtime_init_thread( media_server_thread, 0 , B_MEDIA_REALTIME_ANYKIND);
//if (result_of_locking_server != B_OK)
//	{
	//Give out message that the server could not be locked in memory for realtime performance.
//	}


//find and init audio hardware!=========
//if no audio hardware has been found then it's still not a reason to quit the media server
//because media server serves for audio, video and any other kind of node in the system.
status_t 	result_of_finding_hardware = FindHardware();

if (result_of_finding_hardware != B_OK)
	{
	last_media_server_status = MEDIA_SERVER_NO_AUDIO_HARDWARE;
	}



	
status_t	result_of_initializing_hardware = InitializeHardware();

if (result_of_initializing_hardware != B_OK)
	{
	last_media_server_status = MEDIA_SERVER_FAILED_TO_INITIALIZE_AUDIO_HARDWARE;
	}


//Notifying the world about my start.
BMessage my_start( MEDIA_SERVER_HAS_STARTED );
be_roster->Broadcast( &my_start );

PostMessage(MEDIA_SERVER_OPEN_WINDOW);
}



/*======*/
/*======*/
bool MediaServerApp_Class::QuitRequested( void )
{
const int yes = 0;

int x = Private::YesNoBox("Do you really want to quit the Media Server?");

if (x == yes)
	{
	BMessage my_death( MEDIA_SERVER_HAS_DIED );
	be_roster->Broadcast( &my_death );
	return true;
	}
	
return false;
}



/*======*/
/*======*/
void MediaServerApp_Class::MessageReceived( BMessage *message_to_media_server )
{
BApplication::MessageReceived( message_to_media_server );






//Handling media server communication
switch (message_to_media_server->what)
	{
	
	case MEDIA_SERVER_REGISTER_APP:
		{
		RegisterMediaApplication ( message_to_media_server );
		break;
		}
		
		
		
	case MEDIA_SERVER_UNREGISTER_APP:
		{
		UnRegisterMediaApplication ( message_to_media_server );
		break;
		}
		
		
		
	case MEDIA_SERVER_OPEN_WINDOW:
		{
		if (global_window == 0)
			{
			global_window = new MediaWindow_Class;
			global_window->WindowIsDying->NotifyMe( this ); //Notify the Media Server when window is dying
			global_window->ServerDeathRequest->NotifyMe( this );
			global_window->NewDeviceSelection->NotifyMe( this );
			
			
			AppRegistered->NotifyMe 	( global_window ); //hook the window to be notified
			AppUnregistered->NotifyMe 	( global_window );
			DevicesChanged->NotifyMe	( global_window );
			
			global_window->Show();
			DevicesChanged->NotifyClients( FoundDevices ); //Upload the device list to the window
			}
		else{
			global_window->Activate();
			}
		break;
		}	
		
		
		
	case MEDIA_SERVER_CLOSE_WINDOW:
		{
		AppRegistered->StopNotifyingMe ( global_window );
		AppUnregistered->StopNotifyingMe ( global_window );
		DevicesChanged->StopNotifyingMe ( global_window );
		
		global_window->Lock();
		global_window->Quit();
		global_window = 0;
		}	
	}
}



/*======*/
/*======*/
status_t MediaServerApp_Class::FindHardware( void )
{
int i = DeviceDetection::FindDevices( FoundDevices );
if (i > 0) return B_OK;

return B_ERROR;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::InitializeHardware( void )
{
return B_ERROR;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::StopServer( void )
{
if (media_server_has_started)
	PostMessage( B_QUIT_REQUESTED );
	
return B_ERROR; //if we are not gone yet, then something bad must have happened!
}



/*======*/
/*======*/
int MediaServerApp_Class::DeviceCount( void )
{
return -1;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::UseDevice ( int device_number )
{
int devices = FoundDevices->CountItems();

if ((device_number >= 0)&&(device_number < devices))
	{
	used_device_id = device_number;
	return B_OK;
	}

return B_ERROR;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::LoadSettingsFromDisk	( void )
{
BPath *PathToSettingsFile = 0;
status_t 	directory_found = find_directory( B_COMMON_SETTINGS_DIRECTORY, PathToSettingsFile, true);
status_t	outcome_of_this = B_OK;

if (directory_found == B_OK)
	{
	PathToSettingsFile->Append( MEDIA_SERVER_SETTINGS_FILE );
	BFile settings_file( PathToSettingsFile->Path(), B_READ_ONLY | B_CREATE_FILE );
	if (settings_file.InitCheck() == B_OK)
		{
		off_t file_size;
		char file_buf[1000];
		char file_line[1000];
		off_t file_pos = 0;
		int line_length = 0;
		bool satisfied = false;
		int actual_size;
					
		settings_file.GetSize( &file_size);

		while (!satisfied)
			{
			actual_size = settings_file.ReadAt(file_pos, &file_buf, 1000);
			line_length = Private::CopyLine(file_buf, file_line, actual_size);
			file_line[line_length] = 0;
		
			if (Private::StringFitsRule(file_line, RULE_SETTING_MASTERVOLUME) == true)
				{
				//Wert auslesen
				}
				
			file_pos += line_length;
			}
		}
	else{
		//Set default values
		}
	}
else{
	outcome_of_this = B_ERROR;
	}

return outcome_of_this;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::RegisterMediaApplication 	( BMessage *registration_message )
{
team_id *id = new team_id;
status_t success;

success = registration_message->FindInt32("team", id);
if (success == B_OK)
	{
	Private::OKBox("A new Application has registered!");
	registration_message->SendReply( &BMessage( MEDIA_SERVER_REGISTER_APP ));
	
	RegisteredApplications->AddItem( id );
	AppRegistered->NotifyClients( RegisteredApplications );
	
	return B_OK;
	}
	
return B_ERROR;
}



/*======*/
/*======*/
status_t MediaServerApp_Class::UnRegisterMediaApplication	( BMessage *registration_message )
{
team_id 	id;
team_id 	temp_id;
status_t 	success;

success = registration_message->FindInt32("team", &id);
if (success == B_OK)
	{
	registration_message->SendReply( &BMessage( MEDIA_SERVER_UNREGISTER_APP ));
	Private::OKBox("An Application has unregistered!");
	
	int i,c;
	c = RegisteredApplications->CountItems();
	
	//There can be multiple registrations!
	for (i = c-1; i >= 0; --i)
		{
		temp_id = *((team_id *) RegisteredApplications->ItemAt(i));
		if (temp_id == id)
			{
			RegisteredApplications->RemoveItem(i);
			}
		}
	
	AppUnregistered->NotifyClients( RegisteredApplications );
	return B_OK;
	}

return B_ERROR;
}



/*======*/
/*======*/
void MediaServerApp_Class::OnClientNotified( void *data , int ID)
{
if (ID == EVENT_WINDOW_IS_DYING)
	{
	global_window = NULL;
	}
if (ID == EVENT_SERVER_DEATH_REQUESTED)
	{
	StopServer();
	}
if (ID == EVENT_DEVICE_CHANGED)
	{
	int x = *( (int*) data );
	UseDevice( x );
	}
}
//END OF MediaServerApp_Class=====================================================
//================================================================================
//================================================================================


















//================================================================================
//================================================================================
//MAIN ===========================================================================
int main()
{
last_media_server_status = MEDIA_SERVER_NORMAL_OPERATION;

// if you want to check for crucial things before starting then do this here.
// if you find a show stopper then set the appropriate last_media_server_status.

if (last_media_server_status == MEDIA_SERVER_NORMAL_OPERATION)
	{
	media_server = new MediaServerApp_Class();
	media_server->Run();
	}

return last_media_server_status;
}
//====================================================================