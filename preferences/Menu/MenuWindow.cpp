	#include "MenuApp.h"
	
	int ans;
	
	MenuWindow::MenuWindow()
		: BWindow(rect, "Menu", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
	{
	 	get_menu_info(&revert_info);
	 	get_menu_info(&info);
	 	revert = false;
	 	MoveTo((rect.left += 100),(rect.top += 100));
	 	menuView = new MenuView();
		menuBar = new MenuBar();
		fontMenu = new FontMenu();
		fontSizeMenu = new FontSizeMenu();
		menuBar->AddItem(fontMenu, 0);
		menuBar->AddItem(fontSizeMenu, 1);
		menuView->AddChild(menuBar);
		AddChild(menuView);
		menuView->ResizeTo((Frame().right),(Frame().bottom));
		defaultButton = new BButton(BRect(10,0,85,20), "Default", "Defaults",
			new BMessage(MENU_DEFAULT), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
		revertButton = new BButton(BRect(95,0,175,20), "Revert", "Revert",
			new BMessage(MENU_REVERT), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
		revertButton->SetEnabled(false);
		
		menuView->AddChild(defaultButton);
		menuView->AddChild(revertButton);
		
	}
	
	MenuWindow::~MenuWindow()
	{/*nothing to delete;*/}
	
	void
	MenuWindow::MessageReceived(BMessage *msg)
	{
		revert = true;
		switch(msg->what) {
		
		case MENU_REVERT:
			set_menu_info(&revert_info);
			revert = false;
			Update();
			break;
			
		case MENU_DEFAULT:
			Defaults();
			break;
			
		case UPDATE_WINDOW:
			Update();
			break;
			
		case FONT_SIZE_NINE:{
			get_menu_info(&info);
			info.font_size = 9;
			set_menu_info(&info);
			Update();
			break;}
		
		case FONT_SIZE_TEN:{
			get_menu_info(&info);
			info.font_size = 10;
			set_menu_info(&info);
			Update();
			break;}
		
		case FONT_SIZE_ELEVEN:{
			get_menu_info(&info);
			info.font_size = 11;
			set_menu_info(&info);
			Update();
			break;}
		
		case FONT_SIZE_TWELVE:{
			get_menu_info(&info);
			info.font_size = 12;
			set_menu_info(&info);
			Update();
			break;}
		
		case FONT_SIZE_FOURTEEN:{
			get_menu_info(&info);
			info.font_size = 14;
			set_menu_info(&info);
			Update();
			break;}
				
		case FONT_SIZE_EIGHTEEN:{
			get_menu_info(&info);
			info.font_size = 18;
			set_menu_info(&info);
			Update();
			break;}
			
		case SEP_ZERO:
			get_menu_info(&info);
			info.separator = 0;
			set_menu_info(&info);
			Update();
			break;
		
		case SEP_ONE:
			get_menu_info(&info);
			info.separator = 1;
			set_menu_info(&info);
			Update();
			break;
		
		case SEP_TWO:
			get_menu_info(&info);
			info.separator = 2;
			set_menu_info(&info);
			Update();
			break;
		
		case CLICK_OPEN_MSG:
			get_menu_info(&info);
			if (info.click_to_open != true)
				info.click_to_open = true;
			else
				info.click_to_open = false;
			set_menu_info(&info);
			menuBar->set_menu();
			break;
		
		case ALLWAYS_TRIGGERS_MSG:
			get_menu_info(&info);
			if (info.triggers_always_shown != true)
				info.triggers_always_shown = true;
			else
				info.triggers_always_shown = false;
			set_menu_info(&info);
			menuBar->set_menu();
			break;
		
		case CTL_MARKED_MSG:
			if(menuBar->ctlAsShortcutItem->IsMarked() == true){break;}
			menuBar->toggle_key_marker();
			break;
		
		case ALT_MARKED_MSG:
			if(menuBar->altAsShortcutItem->IsMarked() == true){break;}
			menuBar->toggle_key_marker();
			break;
		
		case COLOR_SCHEME_MSG:
			colorWindow = new ColorWindow();
			colorWindow->Show();
			break;
		
		case MENU_COLOR:
			set_menu_info(&info);
			(new BAlert("test","we made it","cool"))->Go();
			break;
		
		default:
			BMessage(msg);
			break;
		}
	}
	
	bool
	MenuWindow::QuitRequested()
	{
		be_app->PostMessage(B_QUIT_REQUESTED);
		return true;
	}
	
	void
	MenuWindow::Update()
	{
    	revertButton->SetEnabled(revert);
    
    	// alert the rest of the application to update	
		fontMenu->Update();
		fontSizeMenu->Update();
		menuBar->Update();
		
		// resize the window according to the size of menuBar
		ResizeTo((menuBar->Frame().right + 35), (menuBar->Frame().bottom + 45));
	}
	
	void
	MenuWindow::Defaults()
	{
		// to set the default color. this should be changed 
		// to the system color for system wide compatability.
		rgb_color color;
		color.red = 219;
		color.blue = 219;
		color.green = 219;
		color.alpha = 255;
	
		// the default settings. possibly a call to the app_server
		// would provide and execute this information, as it does
		// for get_menu_info and set_menu_info (or is this information
		// coming from libbe.so? or else where?). 
		info.font_size = 12;
		//info.f_family;
		//info.f_style;
		info.background_color = color;
		info.separator = 0;
		info.click_to_open = true;
		info.triggers_always_shown = false;
		set_menu_info(&info);
		Update();
	}