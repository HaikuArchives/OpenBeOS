/*
 * A BStringItem modified such that it holds
 * the BEntry object it corresponds with
 */
 
#include "KeymapListItem.h"


KeymapListItem::KeymapListItem( BEntry *_keymap )
	: BStringItem( "" )
{
	char	name[B_FILE_NAME_LENGTH];
	
	keymap = _keymap;
	_keymap->GetName( name );
	
//	name = "boe";
	SetText( name );
//	delete name;
}

KeymapListItem::~KeymapListItem()
{
	delete keymap;
}

BEntry* KeymapListItem::KeymapEntry()
{
	return keymap;
}
