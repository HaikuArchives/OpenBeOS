#ifndef SERVER_CLIENT_MODEL
#define SERVER_CLIENT_MODEL

#include <List.h>





class Client_Class
	{
	public:
	Client_Class( void );
	virtual void OnClientNotified( void * , int ID);
	};




class Server_Class
	{
	BList *ClientList;
	int MY_ID;
	
	public:
	Server_Class( int ID );
	~Server_Class( void );
	
	void NotifyMe( Client_Class * );
	void StopNotifyingMe( Client_Class *);
	void NotifyClients( void *x = NULL );
	};


#endif