	#include "MenuApp.h"
	
	MenuBar::MenuBar()
		:BMenuBar(BRect(40,10,10,10), "menu", B_FOLLOW_TOP|B_FRAME_EVENTS, B_ITEMS_IN_COLUMN, true)
	{
		get_menu_info(&info);
		build_menu();
		set_menu();
	}
	
	MenuBar::~MenuBar()
	{ /*nothing to clean up*/}
	
	void
	MenuBar::build_menu()
	{
		
		// create the menu items
		clickToOpenItem = new BMenuItem("Click To Open", new BMessage(CLICK_OPEN_MSG), 0, 0);
		alwaysShowTriggersItem = new BMenuItem("Allways Show Triggers", new BMessage(ALLWAYS_TRIGGERS_MSG), 0, 0);
		separatorStyleItem = new BMenuItem("Separator Style", new BMessage(DEFAULT_MSG), 0, 0);
		ctlAsShortcutItem = new BMenuItem("CTL As Shortcut Key", new BMessage(CTL_MARKED_MSG), 0, 0);
		altAsShortcutItem = new BMenuItem("ALT As Shortcut Key", new BMessage(ALT_MARKED_MSG), 0, 0);
		altAsShortcutItem->SetMarked(true);
		
		// color menu
		colorSchemeItem = new BMenuItem("Color Scheme...", new BMessage(COLOR_SCHEME_MSG), 0, 0);
	
		// create the separator menu
		separatorStyleMenu = new BMenu("Separator Style", B_ITEMS_IN_COLUMN);
		separatorStyleMenu->SetRadioMode(true);
		separatorStyleZero = new BMenuItem("Separator Style 0", new BMessage(SEP_ZERO), 0, 0);
		separatorStyleOne = new BMenuItem("Separator Style 1", new BMessage(SEP_ONE), 0, 0);
		separatorStyleTwo = new BMenuItem("Separator Style 2", new BMessage(SEP_TWO), 0, 0);
		if(info.separator == 0)
			{separatorStyleZero->SetMarked(true);}
		if(info.separator == 1)
			{separatorStyleOne->SetMarked(true);}
		if(info.separator == 2)
			{separatorStyleTwo->SetMarked(true);}
		separatorStyleMenu->AddItem(separatorStyleZero);
		separatorStyleMenu->AddItem(separatorStyleOne);
		separatorStyleMenu->AddItem(separatorStyleTwo);
		separatorStyleMenu->SetTargetForItems(Window());
	
		// Add items to menubar	
		AddSeparatorItem();
		AddItem(clickToOpenItem);
		AddItem(alwaysShowTriggersItem);
		AddSeparatorItem();
		AddItem(colorSchemeItem);
		AddItem(separatorStyleMenu);
		AddSeparatorItem();
		AddItem(ctlAsShortcutItem);
		AddItem(altAsShortcutItem);
		SetTargetForItems(Window());
	}
	
	void
	MenuBar::toggle_key_marker()
	{
		if (ctlAsShortcutItem->IsMarked()) {
			ctlAsShortcutItem->SetMarked(false);
			ctlAsShortcutItem->SetEnabled(true);
			
			altAsShortcutItem->SetMarked(true);
			altAsShortcutItem->SetEnabled(true);
		} else {
			altAsShortcutItem->SetMarked(false);
			altAsShortcutItem->SetEnabled(true);
			
			ctlAsShortcutItem->SetMarked(true);
			ctlAsShortcutItem->SetEnabled(true);
		}
		
		set_menu_info(&info);
	}
	
	void
	MenuBar::set_menu()
	{
		// make sure that the info being used is 
		// up-to-date
		get_menu_info(&info);
		
		if (info.triggers_always_shown) {
			alwaysShowTriggersItem->SetMarked(true);
			alwaysShowTriggersItem->SetEnabled(true);	
		} else {
			alwaysShowTriggersItem->SetMarked(false);
			alwaysShowTriggersItem->SetEnabled(true);	
		}
	
		if (info.click_to_open) {
			clickToOpenItem->SetMarked(true);
			clickToOpenItem->SetEnabled(true);
			
			alwaysShowTriggersItem->SetEnabled(true);
		} else {
			clickToOpenItem->SetMarked(false);
			clickToOpenItem->SetEnabled(true);
			
			alwaysShowTriggersItem->SetEnabled(false);
		}	
		
		set_menu_info(&info);
	}
	
	void
	MenuBar::Update()
	{
		// get up-to-date menu info
		get_menu_info(&info);

		// this needs to be updated incase the Defaults
		// were requested.
		if (info.separator == 0)
			separatorStyleZero->SetMarked(true);
		else if (info.separator == 1)
			separatorStyleOne->SetMarked(true);
		else if (info.separator == 2)
			separatorStyleTwo->SetMarked(true);
		
		BFont font;
		Window()->Lock();
 		font.SetFamilyAndStyle(info.f_family, info.f_style);
 		font.SetSize(info.font_size);
 		SetFont(&font);
 		SetViewColor(info.background_color);
		Window()->Unlock();
	
		// force the menu to redraw
		InvalidateLayout();
	}
	
	void MenuBar::FrameResized(float width, float height)
	{
		Window()->PostMessage(UPDATE_WINDOW);	
	}