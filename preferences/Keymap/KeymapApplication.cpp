#include <be/add-ons/input_server/InputServerDevice.h>
#include <be/storage/File.h>
#include <be/app/Application.h>
#include <be/storage/Directory.h>
#if DEBUG
	#include <be/interface/Input.h>
	#include <iostream.h>
#endif //DEBUG
#include "KeymapWindow.h"
#include "KeymapApplication.h"


KeymapApplication::KeymapApplication()
	:	BApplication( APP_SIGNATURE )
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
		// put each files' name in the list
		while( true ) {
			currentEntry = new BEntry();
			if( directory->GetNextEntry( currentEntry, true ) != B_OK ) {
				delete currentEntry;
				break;
			}
			entryList->AddItem( currentEntry );
			#if DEBUG
				char	name[B_FILE_NAME_LENGTH];
				currentEntry->GetName( name );
				cout << "Found: " << name << endl;
			#endif //DEBUG
		}
	else {
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

	// Open input file
	if( !keymap->Exists() )
		return false;
	inFile = new BFile( keymap, B_READ_ONLY );
	if( !inFile->IsReadable() ) {
		delete inFile;
		return false;
	}
	
	// Is keymap a valid keymap file?
	if( !IsValidKeymap( inFile ) ) {
		delete inFile;
		return false;
	}
	
	// Open output file
	// TODO: find a nice constant for the path and use that instead
	outFile = new BFile( "/boot/home/config/settings/Key_map", B_WRITE_ONLY|B_ERASE_FILE|B_CREATE_FILE );
	if( !outFile->IsWritable() ) {
		delete inFile;
		delete outFile;
		return false;
	}

	// Copy file
	if( Copy( inFile, outFile ) != B_OK ) {
		delete inFile;
		delete outFile;
		return false;
	}
	delete inFile;
	delete outFile;
	
	// Tell Input Server there is a new keymap
	if( NotifyInputServer() != B_OK )
		return false;
	
	return true;
}

bool KeymapApplication::IsValidKeymap( BFile *inFile )
{
	// TODO: implement this thing

	return true;
}

int KeymapApplication::Copy( BFile *original, BFile *copy )
{
	char	*buffer[ COPY_BUFFER_SIZE ];
	int 	offset = 0;
	int		errorCode = B_NO_ERROR;
	ssize_t	nrBytesRead;
	ssize_t	nrBytesWritten;

	while( true ) {
		nrBytesRead = original->ReadAt( offset, buffer, COPY_BUFFER_SIZE );
		if( nrBytesRead == 0 )
		{
			errorCode = B_OK;
			break;
		}
		nrBytesWritten = copy->WriteAt( offset, buffer, nrBytesRead );
		if( nrBytesWritten != nrBytesRead )
		{
			errorCode = B_ERROR;
			break;
		}
		if( nrBytesRead < COPY_BUFFER_SIZE )
		{
			errorCode = B_OK;
			break;
		}
		offset += nrBytesRead;
	}
	
	return errorCode;
}

int KeymapApplication::NotifyInputServer()
{
/*
	BMessage		*message = new BMessage( B_KEY_MAP_CHANGED );

	BInputDevice::Control( B_KEYBOARD_DEVICE, 
		B_KEY_MAP_CHANGED, message );
	delete message;
*/

	// get devices
	void			*currentThingy;
	BInputDevice	*currentDevice;
	BMessage		*message = new BMessage( B_KEY_MAP_CHANGED );
	BList			*devices = new BList();
	int				success;

	if( get_input_devices( devices ) != B_OK ) {
		delete devices;
		delete message;
		return B_ERROR;
	}
	
	// inform all keyboard devices
	int nrDevices = devices->CountItems();
	for( int index=0; index<nrDevices; index++ ) {
		currentThingy = devices->ItemAt( index );
		currentDevice = (BInputDevice*) currentThingy;
		if( currentDevice->Type() == B_KEYBOARD_DEVICE ) {
/*
			#if DEBUG
				cout << "Notifying keyboard: " << currentDevice->Name();
			#endif //DEBUG

			// Spread the word
			success = currentDevice->Control( B_KEY_MAP_CHANGED, message );
			#if DEBUG
				cout << "... " << ( success==B_OK?"succeeded":"failed" ) << endl;
			#endif //DEBUG
*/
			#if DEBUG
				cout << "Notifying keyboard: " << currentDevice->Name() << endl;
			#endif //DEBUG

			currentDevice->Stop();
			success = currentDevice->Control( B_KEY_MAP_CHANGED, message );
			currentDevice->Start();
		}
		delete currentDevice;
	}
	delete devices;
	delete message;

	return B_NO_ERROR;
}
