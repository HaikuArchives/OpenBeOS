#include "KeymapApplication.h"


KeymapApplication::KeymapApplication()
	:BApplication( APP_SIGNATURE )
{
	// create the window
	BRect frame = WINDOW_DIMENSIONS;
	frame.OffsetTo( WINDOW_LEFT_TOP_POSITION );
	window = new KeymapWindow( frame );
	window->Show();	
}

void KeymapApplication::MessageReceived( BMessage * message )
{

}


/*
 * Returns a BList containing BEntry objects
 * representing the systems' keymap files
 */
BList* KeymapApplication::SystemMaps()
{
	// TODO: find a constant containing this path and use that instead
	return EntryList( "/boot/beos/etc/Keymap" );
}

BList* KeymapApplication::UserMaps()
{
	// TODO: find a constant containing this path and use that instead
	return EntryList( "/boot/home/config/settings/Keymap" );
}

BList*	KeymapApplication::EntryList( char *directoryPath )
{
	BList		*entryList;
	BDirectory	*directory;
	BEntry		*currentEntry;
	#ifdef DEBUG
		char	name[B_FILE_NAME_LENGTH];
	#endif //DEBUG


	entryList = new BList();
	directory = new BDirectory();

	if( directory->SetTo( directoryPath ) == B_OK )
	{
		// put each file's name in the list
		while( true )
		{
			currentEntry = new BEntry();
			if( directory->GetNextEntry( currentEntry, true ) != B_OK )
			{
				#ifdef DEBUG
//					delete name;
				#endif //DEBUG
				delete currentEntry;
				break;
			}
			entryList->AddItem( currentEntry );
			#ifdef DEBUG
				currentEntry->GetName( name );
				cout << "Found: " << name << endl;
			#endif //DEBUG
		}
	}
	else
	{
		// something went wrong; no system keymaps today
		// TODO: catch error codes and act appropriately
		
	}
	delete directory;

	return entryList;
}

BEntry* KeymapApplication::CurrentMap()
{
	BEntry		*entry;

	// TODO: find a constant containing this path and use that instead
	entry = new BEntry( "/boot/home/config/settings/Key_map" );
	return entry;
}
