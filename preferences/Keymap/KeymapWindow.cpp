#include <be/app/Application.h>
#include <be/interface/View.h>
#include <be/interface/GraphicsDefs.h>
#include <be/interface/MenuItem.h>
#include <be/interface/ListView.h>
#include <be/interface/ScrollView.h>
#include <be/interface/Button.h>
#include <be/interface/Alert.h>
#include <be/interface/Box.h>
#ifdef DEBUG
	#include <iostream.h>
#endif //DEBUG
#include "KeymapWindow.h"
#include "KeymapListItem.h"
#include "KeymapApplication.h"
#include "messages.h"

#include <iostream.h>

KeymapWindow::KeymapWindow( BRect frame )
	:	BWindow( frame, WINDOW_TITLE, B_TITLED_WINDOW,
			B_NOT_ZOOMABLE|B_NOT_RESIZABLE|B_ASYNCHRONOUS_CONTROLS )
{
	rgb_color	temp_color;
	BRect		bounds = Bounds();
	BMenuBar	*menubar;

	fApplication = (KeymapApplication*) be_app;
	fSelectedMap = fApplication->CurrentMap();
	
	// Add the menu bar
	menubar = AddMenuBar();

	// The view to hold all but the menu bar
	bounds.top = menubar->Bounds().bottom + 1;
	fPlaceholderView = new BView( bounds, "placeholderView", 
		B_FOLLOW_NONE, 0 );
	temp_color = ui_color( B_MENU_BACKGROUND_COLOR );
	fPlaceholderView->SetViewColor( temp_color );
	AddChild( fPlaceholderView );

	// Create the Maps box and contents
	AddMaps();
	
	// The 'Use' button
	bounds.Set( 527,200, 600,220 );
	fUseButton = new BButton( bounds, "useButton", "Use",
		new BMessage( USE_KEYMAP ));
	fPlaceholderView->AddChild( fUseButton );
	
}

BMenuBar * KeymapWindow::AddMenuBar()
{
	BRect		bounds;
	BMenu		*menu;
	BMenuItem	*currentItem;
	BMenuBar	*menubar;

	bounds = Bounds();
	menubar = new BMenuBar( bounds, "menubar" );
	AddChild(menubar);
	
	// Create the File menu
	menu = new BMenu( "File" );
	menu->AddItem( new BMenuItem( "Open" B_UTF8_ELLIPSIS,
		new BMessage( MENU_FILE_OPEN ), 'O' ) );
	menu->AddSeparatorItem();
	currentItem = new BMenuItem( "Save",
		new BMessage( MENU_FILE_SAVE ), 'S' );
	currentItem->SetEnabled( false );
	menu->AddItem( currentItem );
	menu->AddItem( new BMenuItem( "Save As" B_UTF8_ELLIPSIS,
		new BMessage( MENU_FILE_SAVE_AS )));
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Quit",
		new BMessage( B_QUIT_REQUESTED ), 'Q' ));
	menubar->AddItem( menu );

	// Create the Edit menu
	menu = new BMenu( "Edit" );
	currentItem = new BMenuItem( "Undo",
		new BMessage( MENU_EDIT_UNDO ), 'Z' );
	currentItem->SetEnabled( false );
	menu->AddItem( currentItem );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Cut",
		new BMessage( MENU_EDIT_CUT ), 'X' ));
	menu->AddItem( new BMenuItem( "Copy",
		new BMessage( MENU_EDIT_COPY ), 'C' ));
	menu->AddItem( new BMenuItem( "Paste",
		new BMessage( MENU_EDIT_PASTE ), 'V' ));
	menu->AddItem( new BMenuItem( "Clear",
		new BMessage( MENU_EDIT_CLEAR )));
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Select All",
		new BMessage( MENU_EDIT_SELECT_ALL ), 'A' ));
	menubar->AddItem( menu );
	
	// Create the Font menu
	menu = new BMenu( "Font" );
	
	menubar->AddItem( menu );
	
	return menubar;
}

void KeymapWindow::AddMaps()
{
	BBox		*mapsBox;
	BRect		bounds;
	BList		*entryList;
	BList		*listItems;
	KeymapListItem	*currentKeymapItem;

	// The Maps box
	bounds = BRect( 9,11, 140, 227 );
	mapsBox = new BBox( bounds );
	mapsBox->SetLabel( "Maps" );
	fPlaceholderView->AddChild( mapsBox );

	// The System list
	mapsBox->DrawString( "System", BPoint( 13, 20 ) );
	bounds = BRect( 13,36, 103,106 );
	entryList = fApplication->SystemMaps();
	fSystemListView = new BListView( bounds, "systemList" );
	listItems = ListItemsFromEntryList( entryList );
	fSystemListView->AddList( listItems );
	mapsBox->AddChild( new BScrollView( "systemScrollList", fSystemListView,
		B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true ));
	fSystemListView->SetSelectionMessage( new BMessage( SYSTEM_MAP_SELECTED ));
	delete listItems;
	delete entryList;

	// The User list
	mapsBox->DrawString( "User", BPoint( 13, 113 ));
	bounds = BRect( 13,129, 103,199 );
	entryList = fApplication->UserMaps();
	fUserListView = new BListView( bounds, "userList" );
	// '(Current)'
	currentKeymapItem = ItemFromEntry( fApplication->CurrentMap() );
	if( currentKeymapItem != NULL )
		fUserListView->AddItem( currentKeymapItem );
	// Saved keymaps
	listItems = ListItemsFromEntryList( entryList );
	fUserListView->AddList( listItems );
	mapsBox->AddChild( new BScrollView( "systemScrollList", fUserListView,
		B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true ));
	fUserListView->SetSelectionMessage( new BMessage( USER_MAP_SELECTED ));
	delete listItems;
	delete entryList;
}

BList* KeymapWindow::ListItemsFromEntryList( BList * entryList)
{
	BEntry			*currentEntry;
	BList			*listItems;
	int				nrItems;
	#ifdef DEBUG
		char	name[B_FILE_NAME_LENGTH];
	#endif //DEBUG

	listItems = new BList();
	nrItems = entryList->CountItems();
	for( int index=0; index<nrItems; index++ ) {
		currentEntry = (BEntry*)entryList->ItemAt( index );
		listItems->AddItem( new KeymapListItem( currentEntry ));
		
		#ifdef DEBUG
			currentEntry->GetName( name );
			cout << "New list item: " << name << endl;
		#endif //DEBUG
	}

	return listItems;
}

KeymapListItem* KeymapWindow::ItemFromEntry( BEntry *entry )
{
	KeymapListItem	*item;

	if( entry->Exists() ) {
		item = new KeymapListItem( entry );
		item->SetText( "(Current)" );
	}
	else
		item = NULL;

	return item;
}

bool KeymapWindow::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void KeymapWindow::MessageReceived( BMessage* message )
{
	switch( message->what ) {
		case MENU_FILE_OPEN:
			break;
		case MENU_FILE_SAVE:
			break;
		case MENU_FILE_SAVE_AS:
			break;
		case MENU_EDIT_UNDO:
			break;
		case MENU_EDIT_CUT:
			break;
		case MENU_EDIT_COPY:
			break;
		case MENU_EDIT_PASTE:
			break;
		case MENU_EDIT_CLEAR:
			break;
		case MENU_EDIT_SELECT_ALL:
			break;
		case SYSTEM_MAP_SELECTED:
			HandleSystemMapSelected( message );
			break;
		case USER_MAP_SELECTED:
			HandleUserMapSelected( message );
			break;
		case USE_KEYMAP:
			UseKeymap();
			break;

		default:	
			BWindow::MessageReceived( message );
			break;
	}
}

void KeymapWindow::HandleSystemMapSelected( BMessage *selectionMessage )
{
	cout << "System map selected" << endl;
	HandleMapSelected( selectionMessage, fSystemListView, fUserListView );
}

void KeymapWindow::HandleUserMapSelected( BMessage *selectionMessage )
{
	cout << "User map selected" << endl;
	HandleMapSelected( selectionMessage, fUserListView, fSystemListView );
}

void KeymapWindow::HandleMapSelected( BMessage *selectionMessage,
	BListView * selectedView, BListView * otherView )
{
	// Deselect current other map
	int32	otherIndex;

	otherIndex = otherView->CurrentSelection();
/*	if( otherIndex >= 0 )
	{
		int32	selectedIndex;
		
		selectedIndex = selectedView->CurrentSelection();
*/
		otherView->Deselect( otherIndex );
/*//		otherView->InvalidateItem( otherIndex );
		selectedView->Select( selectedIndex );		
		selectedView->InvalidateItem( selectedIndex );
	}
*/
	// save entry for selected map in fSelectedMap
	int32			index;
	KeymapListItem	*keymapListItem;

	index = selectedView->CurrentSelection( 0 );
	if( index < 0 )
		return;
	keymapListItem = (KeymapListItem*)selectedView->ItemAt( index );
	if( keymapListItem == NULL )
		return;
	fSelectedMap = keymapListItem->KeymapEntry();
	#if DEBUG
		char	name[B_FILE_NAME_LENGTH];
		
		fSelectedMap->GetName( name );
		cout << "fSelectedMap has been set to " << name << endl;
	#endif //DEBUG

	// Display selected map

}

void KeymapWindow::UseKeymap()
{
	if( fSelectedMap != NULL )
		fApplication->UseKeymap( fSelectedMap );
	else {
		// There is no keymap selected!
		BAlert	*alert;
		alert = new BAlert( "w>noKeymap", "No keymap has been selected", "Bummer",
			NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT );
		alert->Go();
	}
}
