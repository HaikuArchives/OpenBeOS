






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
  #        MODULE: Server Window Module Nov 2001                                #
\*###############################################################################*/








#include "MediaWindow.h"
#include <Application.h>
#include "PrivateFunctionsModule.h"

#define QUIT_SERVER 1


MediaWindow_Class::MediaWindow_Class():
BWindow(BRect(20,20,270,170),"Media Server", B_TITLED_WINDOW, 
B_NOT_RESIZABLE | B_NOT_ANCHORED_ON_ACTIVATE | B_ASYNCHRONOUS_CONTROLS |
B_NOT_ZOOMABLE | B_WILL_ACCEPT_FIRST_CLICK)
{
WindowIsDying 		= new Server_Class ( EVENT_WINDOW_IS_DYING );
ServerDeathRequest 	= new Server_Class ( EVENT_SERVER_DEATH_REQUESTED );
NewDeviceSelection 	= new Server_Class ( EVENT_DEVICE_CHANGED );

Background = new BView( BRect(0,0,250,150), "Background", B_FOLLOW_ALL_SIDES,  0);
AddChild( Background);
Background->SetViewColor(200,200,200);

QuitButton = new BButton( BRect(10,120,240,145), "QuitButton","Stop Media Server", new BMessage( QUIT_SERVER ));
Background->AddChild( QuitButton );

RegisteredTeams = new BListView( BRect(5,5,70,115), "Reg. Teams");
Background->AddChild( new BScrollView("scroll reg. teams", RegisteredTeams,0,0,false, true ));

DeviceList = new BListView( BRect(90,5,230,100), "Devices");
Background->AddChild( new BScrollView("scroll devices", DeviceList,0,0,true, true ));
}

MediaWindow_Class::~MediaWindow_Class()
{
WindowIsDying->NotifyClients();
delete WindowIsDying;

delete ServerDeathRequest;

QuitButton->RemoveSelf();
delete QuitButton;

Background->RemoveSelf();
delete Background;
}

void MediaWindow_Class::MessageReceived( BMessage *window_message )
{
switch (window_message->what)
	{
	case QUIT_SERVER:
		{
		ServerDeathRequest->NotifyClients();
		break;
		}
	}
}

void MediaWindow_Class::OnClientNotified( void *xlist , int ID)
{
switch (ID)
	{
	case EVENT_APP_REGISTERED:
	case EVENT_APP_UNREGISTERED:
		{
		BList *TeamList = (BList *) xlist; //Represents MediaServerApp_Class::RegisteredApplications
		// here comes code for both actions
		
		int i,c;
		BStringItem *any;
		team_id what;
		
		//RegisteredTeams is a View
		c = RegisteredTeams->CountItems(); //How many items are on the ListView
		for (i=0; i<c; ++i) //deleting all "BStringItem"s
			{
			any = (BStringItem *) RegisteredTeams->ItemAt(i);
			delete any;
			}
			 
		Lock();
		RegisteredTeams->MakeEmpty(); //clearing all reference
		
		c = TeamList->CountItems();
		for (i=0; i<c; ++i)
			{
			what = *( (team_id*) TeamList->ItemAt(i));
			RegisteredTeams->AddItem( new BStringItem( Private::IntegerToString( what ).String() ));
			}
		Unlock();
		break;
		}
	case EVENT_UPDATE_DEVICE_LIST:
		{
		BList *lx = (BList *) xlist;
		int i,c;
		BString *device;
		BStringItem *any;

		c = DeviceList->CountItems();
		Lock();
		for (i=0; i<c; ++i) //deleting all "BStringItem"s
			{
			any = (BStringItem *) DeviceList->ItemAt(i);
			delete any;
			}

		c = lx->CountItems();		
		for (i=0; i<c; ++i)
			{
			device = (BString *) lx->ItemAt(i);
			DeviceList->AddItem( new BStringItem( device->String() ) );
			}
			
		Unlock();		
		break;
		}
	}
}
