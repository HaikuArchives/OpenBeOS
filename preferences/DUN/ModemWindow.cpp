/*

ModemWindow by Sikosis (beos@gravity24hr.com)

(C) 2002 OpenBeOS under MIT license

*/

#include "app/Application.h"
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Font.h>
#include <ListItem.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <OutlineListView.h>
#include <stdio.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>
#include <View.h>

#include "DUN.h"
//#include "DUNWindow.h"
#include "ModemWindow.h"
#include "DUNView.h"

// Constants
const uint32 BTN_MODEM_WINDOW_CANCEL = 'BMWC';
const uint32 BTN_MODEM_WINDOW_CUSTOM = 'BMWU';
const uint32 BTN_MODEM_WINDOW_DONE = 'BMWD';

// ModemWindow -- constructor for ModemWindow Class
ModemWindow::ModemWindow(BRect frame) : BWindow (frame, "", B_MODAL_WINDOW, B_NOT_RESIZABLE , 0)
{
   InitWindow();
   Show();
}
// ------------------------------------------------------------------------------- //

// ModemWindow::InitWindow -- Initialization Commands here
void ModemWindow::InitWindow(void)
{
   BRect r;
   r = Bounds();
   
   // Buttons
   BRect btn1(10,r.bottom - 34,83,r.bottom - 16);
   BRect btn2(108,r.bottom - 34,184,r.bottom - 16);
   BRect btn3(196,r.bottom - 34,271,r.bottom - 16);
   btnModemWindowCustom = new BButton(btn1,"Custom","  Custom . . .  ", new BMessage(BTN_MODEM_WINDOW_CUSTOM), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   btnModemWindowCancel = new BButton(btn2,"Cancel","  Cancel  ", new BMessage(BTN_MODEM_WINDOW_CANCEL), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   btnModemWindowDone = new BButton(btn3,"Done","  Done  ", new BMessage(BTN_MODEM_WINDOW_DONE), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    
   // TextView - Your modem is:
   BRect YMILocation (15,20,150,30);
   tvYourModemIs = new BTextView(r, "YourModemIs", YMILocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvYourModemIs->SetText("Your modem is:");
   tvYourModemIs->MakeSelectable(false);
   tvYourModemIs->MakeEditable(false);
   
   // TextView - Connect via:
   BRect CVLocation (28,45,150,55);
   tvConnectVia = new BTextView(r, "ConnectVia", CVLocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvConnectVia->SetText("Connect via:");
   tvConnectVia->MakeSelectable(false);
   tvConnectVia->MakeEditable(false);
   
   // TextView - Labels
   BRect SpdLocation (54,70,150,80);
   tvSpeed = new BTextView(r, "Speed", SpdLocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvSpeed->SetText("Speed:");
   tvSpeed->MakeSelectable(false);
   tvSpeed->MakeEditable(false);
    
    
   // Add Objects to View 
   
   AddChild(btnModemWindowCancel);   
   AddChild(btnModemWindowCustom);   
   AddChild(btnModemWindowDone);   
   AddChild(tvYourModemIs);   
   AddChild(tvConnectVia);   
   AddChild(tvSpeed);   
   
   btnModemWindowDone->MakeDefault(true);
   
   // Add the Drawing View
   AddChild(aModemview = new ModemView(r));
}
// ------------------------------------------------------------------------------- //

// ModemWindow::~ModemWindow -- destructor
ModemWindow::~ModemWindow()
{
   exit(0);
}
// ------------------------------------------------------------------------------- //

// ModemWindow::MessageReceived -- receives messages
void ModemWindow::MessageReceived (BMessage *message)
{
   switch(message->what)
   {
   	   case BTN_MODEM_WINDOW_CANCEL:
			 Hide();
   		   	 break;
       default:
	         BWindow::MessageReceived(message);
    	     break;
   }
}
// end
