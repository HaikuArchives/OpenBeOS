#include <be/add-ons/input_server/InputServerDevice.h>
#include <be/storage/File.h>
#include <be/app/Application.h>
#include <be/storage/Directory.h>
#ifdef DEBUG
	#include <iostream.h>
#endif //DEBUG
#include "KeymapApplication.h"


KeymapApplication::KeymapApplication()
	:BApplication( APP_SIGNATURE )
{
	// create the window
	BRect frame = WINDOW_DIMENSIONS;
	frame.OffsetTo( WINDOW_LEFT_TOP_POSITION );
	fWindow = new KeymapWindow( frame );
	fWindow->Show();	
}

void KeymapApplication::MessageReceived( BMessage * message )
{
	BApplication::MessageReceived( message );
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


	entryList = new BList();
	directory = new BDirectory();

	if( directory->SetTo( directoryPath ) == B_OK )
	{
		// put each files' name in the list
		while( true )
		{
			currentEntry = new BEntry();
			if( directory->GetNextEntry( currentEntry, true ) != B_OK )
			{
				delete currentEntry;
				break;
			}
			entryList->AddItem( currentEntry );
			#ifdef DEBUG
				char	name[B_FILE_NAME_LENGTH];
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

bool KeymapApplication::UseKeymap( BEntry *keymap )
{ // Copies keymap to ~/config/settings/key_map
	BFile	*inFile;
	BFile	*outFile;
	bool	success = false;


	// Open input file
	if( !keymap->Exists() )
	{
		return false;
	}

	inFile = new BFile( keymap, B_READ_ONLY );
	if( !inFile->IsReadable() )
	{
		return false;
	}
	
	// Is keymap a valid keymap file?
	
	// Open output file
	outFile = new BFile( "/boot/home/config/settings/Key_map", B_WRITE_ONLY|B_ERASE_FILE|B_CREATE_FILE );
	if( !outFile->IsWritable() )
	{
		return false;
	}

	// Copy file
	char	*buffer[ COPY_BUFFER_SIZE ];
	int 	offset = 0;
	ssize_t	nrBytesRead;
	ssize_t	nrBytesWritten;

	while( true )
	{
		nrBytesRead = inFile->ReadAt( offset, buffer, COPY_BUFFER_SIZE );
		if( nrBytesRead == 0 )
		{
			success = true;
			break;
		}
		nrBytesWritten = outFile->WriteAt( offset, buffer, nrBytesRead );
		if( nrBytesWritten != nrBytesRead )
		{
			success = false;
			break;
		}
		if( nrBytesRead < COPY_BUFFER_SIZE )
		{
			success = true;
			break;
		}
		
		offset += nrBytesRead;
	}
	delete inFile;
	delete outFile;
	
	// Inform Input Server
	BMessage	*message;
	
	message = new BMessage( B_KEY_MAP_CHANGED );
	// now what?
	
	delete message;

	return success;
}
