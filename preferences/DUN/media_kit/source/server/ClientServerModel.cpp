






/*###############################################################################*\
  #                                                                             #
  #        Open BeOS Server                                                     #
  #        New Media Server                                                     #
  #                                                                             #
  #        Level : Tested                                                       #
  #                                                                             #
  #        Media Kit Team Leader : Marcus Overhagen                             #
  #        Author                : Aleksander Lodwich                           #
  #                                                                             #
  #                                                                             #
  #        MODULE: Module describing Server-Client relationship Nov 2001        #
\*###############################################################################*/
























#include "ClientServerModel.h"


Client_Class::Client_Class( void )
{
}

void Client_Class::OnClientNotified( void *x , int ID)
{
}









Server_Class::Server_Class( int ID )
{
ClientList = new BList();
MY_ID = ID;
}

Server_Class::~Server_Class( void )
{
delete ClientList;
}

void Server_Class::NotifyMe( Client_Class *c)
{
ClientList->AddItem( c );
}

void Server_Class::StopNotifyingMe( Client_Class *client)
{
Client_Class *x = 0;
int i, c;

c = ClientList->CountItems();

for (i=0; i<c; ++i)
	{
	x = static_cast<Client_Class *> (ClientList->ItemAt(i));
	if (x==client)
		{
		ClientList->RemoveItem(i);
		return;
		}
	}
}


void Server_Class::NotifyClients( void *any)
{
int i, c;
Client_Class *x;
c = ClientList->CountItems();

for (i=0; i<c; ++i)
	{
	x = (Client_Class *) ClientList->ItemAt(i);
	x->OnClientNotified( any , MY_ID);
	}

}