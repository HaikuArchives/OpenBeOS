#include "AudioDeviceDetection.h"


class DirectoryScanner_Class
	{
	BDirectory *MyDirectory;
	bool directory;
	public:
	DirectoryScanner_Class(const char *directory_name);
	~DirectoryScanner_Class( void);

	bool IsDirectory( void ) //Inline function
		{
		return directory;
		};
		
	void FindFileItems( BList *list);
	};


DirectoryScanner_Class::DirectoryScanner_Class( const char *directory_name )
{
MyDirectory = new BDirectory( directory_name );
if (MyDirectory->InitCheck() == B_OK)
	directory = true;
	else
	directory = false;
}

DirectoryScanner_Class::~DirectoryScanner_Class( void )
{
delete MyDirectory;
}


void DirectoryScanner_Class::FindFileItems( BList *dirlist)
{
if (!directory) return;

entry_ref 	result_reference;
int 		i, 
			c = MyDirectory->CountEntries();

for (i=0; i<c; ++i)
	{
	MyDirectory->GetNextRef( &result_reference );
	BEntry ENTRY( &result_reference );
	BPath pfad;
	ENTRY.GetPath( &pfad);
	BNode test( pfad.Path());
	
	if (test.InitCheck() == B_OK)
		{
		if (test.IsDirectory())
			{
			DirectoryScanner_Class child_directory( pfad.Path() );		
			child_directory.FindFileItems(dirlist);
			}
		else{
			BString *newitem = new BString( pfad.Path() );
			dirlist->AddItem(newitem);	
			}
		}		
	}
}


int DeviceDetection::FindDevices(BList *DeviceList)
{
int i, c;
BString *path;
c = DeviceList->CountItems();
for (i=0; i<c; ++i)
	{
	path = (BString*) DeviceList->ItemAt(i);
	delete path;
	}

DeviceList->MakeEmpty();
DirectoryScanner_Class dirscan("/dev/audio");
dirscan.FindFileItems( DeviceList );
return DeviceList->CountItems();
}
