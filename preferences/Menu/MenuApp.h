
	#include <Application.h>
	#include <Button.h>
	#include <ColorControl.h>
	#include <Window.h>
	#include <View.h>
	#include <Alert.h>
	#include <Menu.h>
	#include <MenuBar.h>
	#include <MenuItem.h>
	
	#ifndef MENU_H 
	#define MENU_H
	
	#include "msg.h"
	
	class FontSizeMenu : public BMenu {
		public:
						FontSizeMenu();
		virtual			~FontSizeMenu();
		virtual void	Update();
		
		menu_info 		info;
		BMenuItem	*	fontSizeNine;
		BMenuItem	*	fontSizeTen;
		BMenuItem	*	fontSizeEleven;
		BMenuItem	*	fontSizeTwelve;
		BMenuItem	*	fontSizeFourteen;
		BMenuItem	*	fontSizeEighteen;
	};
	
	class FontMenu : public BMenu {
		public:
						FontMenu();
		virtual			~FontMenu();
		virtual void	GetFonts();
		virtual void	Update();
	
		menu_info		info;
		BMenu		*	fontFamily;
		BMenuItem	*	fontStyleItem;
	};
	
	class MenuBar : public BMenuBar {
		public:
						MenuBar();
		virtual 		~MenuBar();
		virtual void	toggle_key_marker();
		virtual void	set_menu();
		virtual void	build_menu();
		virtual void	Update();
		virtual void 	FrameResized(float width, float height);
		
		BRect 			menu_rect;
		BRect			rect;
		menu_info 		info;
		
		//seperator submenu
		BMenu		*	separatorStyleMenu;
		BMenuItem	*	separatorStyleZero;
		BMenuItem	*	separatorStyleOne;
		BMenuItem	*	separatorStyleTwo;
		
		//others
		BMenuItem	*	clickToOpenItem;
		BMenuItem	*	alwaysShowTriggersItem;
		BMenuItem	*	colorSchemeItem;
		BMenuItem	*	separatorStyleItem;
		BMenuItem	*	ctlAsShortcutItem;
		BMenuItem	*	altAsShortcutItem;
	};
	
	class MenuView : public BView {
		public:
						MenuView();
		virtual			~MenuView();
		virtual void	MessageReceived(BMessage *msg);
		
		menu_info		info;
	};
	
	class ColorPicker : public BColorControl {
		public:
						ColorPicker();
		virtual			~ColorPicker();
		virtual	void	MessageReceived(BMessage *msg);
	};
	
	class ColorWindow : public BWindow {
		public:
						ColorWindow();
		virtual			~ColorWindow();
		virtual void	MessageReceived(BMessage *msg);
		
		ColorPicker *	colorPicker;
		BButton 	*	DefaultButton;
		BButton 	*	RevertButton;
		menu_info		revert_info;
		menu_info 		info;
	};
		
	class MenuWindow : public BWindow {
		public:
						MenuWindow();
		virtual			~MenuWindow();
		virtual void	MessageReceived(BMessage *msg);
		virtual bool	QuitRequested();
		virtual	void	Update();
		virtual void	Defaults();
		
				bool	revert;
		ColorWindow	*	colorWindow;
		BMenuItem 	*	toggleItem;
		menu_info		info;
		menu_info		revert_info;
		BRect			rect;	
		BMenu		*	menu;
		MenuBar		*	menuBar;
		MenuView	*	menuView;
		FontMenu	*	fontMenu;
		FontSizeMenu*	fontSizeMenu;
		BButton 	*	revertButton;
		BButton 	*	defaultButton;
	};
	
	class MenuApp : public BApplication {
		public:
						MenuApp();
		virtual			~MenuApp();
		virtual void	Update();
		virtual void	MessageReceived(BMessage *msg);
		virtual bool	QuitRequested();
		
		//main
		MenuWindow	*	menuWindow;
		BRect			rect;
		menu_info		info;
		BMenu 		*	menu;
	};
	
	#endif