#ifndef OBOS_KEYMAP_APPLICATION_H
#define OBOS_KEYMAP_APPLICATION_H

#include <be/storage/Entry.h>
#include <be/support/List.h>
#include "KeymapWindow.h"


#define APP_SIGNATURE		"application/x-vnd.Keymap"
#define COPY_BUFFER_SIZE	1 * 1024


class KeymapApplication : public BApplication
{
	public:
		KeymapApplication();
		void	MessageReceived(BMessage *message);
		BList*	SystemMaps();
		BList*	UserMaps();
		BEntry* CurrentMap();
		bool	UseKeymap( BEntry *keymap );

	protected:
		KeymapWindow	* fWindow;
		
		BList*	EntryList( char *directoryPath );

};

#endif // OBOS_KEYMAP_APPLICATION_H