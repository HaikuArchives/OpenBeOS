
	#include "MenuApp.h"
	
	FontMenu::FontMenu()
		: BMenu("Font", B_ITEMS_IN_COLUMN)
	{
		get_menu_info(&info);
		SetRadioMode(true);
		GetFonts();
	}
	
	FontMenu::~FontMenu()
	{ /*nothing to clean up*/}
	
	void
	FontMenu::GetFonts()
	{
		int32 numFamilies = count_font_families();
		for ( int32 i = 0; i < numFamilies; i++ ) {
			font_family family;
			uint32 flags;
				if ( get_font_family(i, &family, &flags) == B_OK ) {
					fontFamily = new BMenu(family, B_ITEMS_IN_COLUMN);
					AddItem(fontFamily);
					int32 numStyles = count_font_styles(family);
						for ( int32 j = 0; j < numStyles; j++ ) {
							font_style style;
							if ( get_font_style(family, j, &style, &flags) == B_OK ) {
								fontStyleItem = new BMenuItem(style, 
									new BMessage(FONT_MSG), 0, 0);
								SetRadioMode(true);
								fontFamily->AddItem(fontStyleItem);
							}
						}
				}
			
			
		}
	}
	
	void
	FontMenu::Update()
	{
		InvalidateLayout();
		get_menu_info(&info);
		BFont font;
		Supermenu()->Window()->Lock();
 		font.SetFamilyAndStyle(info.f_family, info.f_style);
 		font.SetSize(info.font_size);
 		SetFont(&font);
 		SetViewColor(info.background_color);
 		set_menu_info(&info);
		Supermenu()->Window()->Unlock();
	}