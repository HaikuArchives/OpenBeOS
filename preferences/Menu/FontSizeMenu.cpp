
	#include "MenuApp.h"
	
	FontSizeMenu::FontSizeMenu()
		:BMenu("Font Size", B_ITEMS_IN_COLUMN)
	{		
		get_menu_info(&info);
		
		fontSizeNine = new BMenuItem("9", new BMessage(FONT_SIZE_NINE), 0, 0);
		AddItem(fontSizeNine);
		if(info.font_size == 9){fontSizeNine->SetMarked(true);}
		
		fontSizeTen = new BMenuItem("10", new BMessage(FONT_SIZE_TEN), 0, 0);
		AddItem(fontSizeTen);
		if(info.font_size == 10){fontSizeTen->SetMarked(true);}
		
		fontSizeEleven = new BMenuItem("11", new BMessage(FONT_SIZE_ELEVEN), 0, 0);
		AddItem(fontSizeEleven);
		if(info.font_size == 11){fontSizeEleven->SetMarked(true);}
		
		fontSizeTwelve = new BMenuItem("12", new BMessage(FONT_SIZE_TWELVE), 0, 0);
		AddItem(fontSizeTwelve);
		if(info.font_size == 12){fontSizeTwelve->SetMarked(true);}
		
		fontSizeFourteen = new BMenuItem("14", new BMessage(FONT_SIZE_FOURTEEN), 0, 0);
		AddItem(fontSizeFourteen);
		if(info.font_size == 14){fontSizeFourteen->SetMarked(true);}
		
		fontSizeEighteen = new BMenuItem("18", new BMessage(FONT_SIZE_EIGHTEEN), 0, 0);
		AddItem(fontSizeEighteen);
		if(info.font_size == 18){fontSizeEighteen->SetMarked(true);}
		
		SetTargetForItems(Window());
		SetRadioMode(true);
	}
	
	FontSizeMenu::~FontSizeMenu()
	{ }
	
	void
	FontSizeMenu::Update()
	{
		get_menu_info(&info);
		BFont font;
 		
 		Supermenu()->Window()->Lock();
 		font.SetFamilyAndStyle(info.f_family, info.f_style);
 		font.SetSize(info.font_size);
 		SetFont(&font);
		SetViewColor(info.background_color);
		Supermenu()->Window()->Unlock();
		set_menu_info(&info);
 		
 		InvalidateLayout();
 		Invalidate();
 		SetEnabled(true);
 	}