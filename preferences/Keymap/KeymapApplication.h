#ifndef OBOS_KEYMAP_APPLICATION_H
#define OBOS_KEYMAP_APPLICATION_H

#ifdef DEBUG
	#include <iostream.h>
#endif //DEBUG


#include <be/app/Application.h>
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/support/List.h>
#include "KeymapWindow.h"


#define APP_SIGNATURE	"application/x-vnd.Keymap"


class KeymapApplication : public BApplication
{
	public:
		KeymapApplication();
		void	MessageReceived(BMessage *message);
		BList*	SystemMaps();
		BList*	UserMaps();
		BEntry* CurrentMap();

	protected:
		KeymapWindow	* window;
		
		BList*	EntryList( char *directoryPath );

};

#endif // OBOS_KEYMAP_APPLICATION_H