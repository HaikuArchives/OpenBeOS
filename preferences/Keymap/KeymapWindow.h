#ifndef OBOS_KEYMAP_WINDOW_H
#define OBOS_KEYMAP_WINDOW_H

#ifdef DEBUG
	#include <iostream.h>
#endif //DEBUG

#include <be/app/Application.h>
#include <be/support/List.h>
#include <be/interface/Window.h>
#include <be/interface/View.h>
#include <be/interface/GraphicsDefs.h>
#include <be/interface/MenuBar.h>
#include <be/interface/MenuItem.h>
#include <be/interface/Box.h>
#include <be/interface/ListView.h>
#include <be/interface/ScrollView.h>
#include "KeymapListItem.h"


#ifndef DEBUG
	#define WINDOW_TITLE				"Keymap"
#else
	#define WINDOW_TITLE				"OBOS Keymap"
#endif //DEBUG
#define WINDOW_LEFT_TOP_POSITION	BPoint( 80, 25 )
#define WINDOW_DIMENSIONS			BRect( 0,0, 612,256 )

#define MENU_FILE_OPEN		'mMFO'
#define MENU_FILE_SAVE		'mMFS'
#define MENU_FILE_SAVE_AS	'mMFA'
#define MENU_EDIT_UNDO		'mMEU'
#define MENU_EDIT_CUT		'mMEX'
#define MENU_EDIT_COPY		'mMEC'
#define MENU_EDIT_PASTE		'mMEV'
#define MENU_EDIT_CLEAR		'mMEL'
#define MENU_EDIT_SELECT_ALL 'mMEA'


class KeymapWindow : public BWindow
{
	public:
				KeymapWindow( BRect frame );
		bool	QuitRequested();
		void	MessageReceived( BMessage* message );
	
	protected:
		BView		*placeholderView;
		BListView	*fSystemListView;
		BListView	*fUserListView;
		const char	*title;
		
		BMenuBar		*AddMenuBar();
		void			AddMaps();
		BList			*ListItemsFromEntryList( BList * entryList);
		KeymapListItem* ItemFromEntry( BEntry *entry );

};

#endif // OBOS_KEYMAP_WINDOW_H
