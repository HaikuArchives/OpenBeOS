

	#include "MenuApp.h"
	
	MenuView::MenuView()
		:BView(BRect(0,0,1000,1000), "menuView",
			B_FOLLOW_ALL, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
	{
		SetViewColor(219,219,219,255);
	}
	
	MenuView::~MenuView()
	{
		//nothing to delete
	}
	
	void
	MenuView::MessageReceived(BMessage *msg){
		switch(msg->what) {
			
		default:
			BMessage(msg);
			break;
		}
	}