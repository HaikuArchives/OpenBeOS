/*

DUNWindow by Sikosis (beos@gravity24hr.com)

(C) 2002 OpenBeOS under MIT license

*/

#include "app/Application.h"
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
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
#include "DUNWindow.h"
#include "DUNView.h"

// Constants
const uint32 BTN_MODEM = 'Modm';
const uint32 BTN_DISCONNECT = 'Disc';
const uint32 BTN_CONNECT = 'Conn';
const uint32 CHK_CALLWAITING = 'Ckcw';
const uint32 CHK_DIALOUTPREFIX = 'Ckdp';

const uint32 MENU_NEW = 'MNew';
const uint32 MENU_DELETE_CURRENT = 'MDel';

// DUNWindow -- constructor for DUNWindow Class
DUNWindow::DUNWindow(BRect frame) : BWindow (frame, "OBOS Dial-up Networking", B_TITLED_WINDOW, B_NOT_RESIZABLE , 0) {
   InitWindow();
   Show();
}
// ------------------------------------------------------------------------------- //

// DUNWindow::_InitWindow -- Initialization Commands here
void DUNWindow::InitWindow(void) {
   BRect r;
   r = Bounds();
   
   // Buttons
   float ButtonTop = 217;
   float ButtonWidth = 24;
   BRect btn1(10,ButtonTop,83,ButtonTop+ButtonWidth);
   modembutton = new BButton(btn1,"Modem","Modem...", new BMessage(BTN_MODEM), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   BRect btn2(143,ButtonTop,218,ButtonTop+ButtonWidth);
   disconnectbutton = new BButton(btn2,"Disconnect","Disconnect", new BMessage(BTN_DISCONNECT), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   BRect btn3(230,ButtonTop,302,ButtonTop+ButtonWidth);
   connectbutton = new BButton(btn3,"Connect","Connect", new BMessage(BTN_CONNECT), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   
   // Check Boxes -- Only when form in state 2
   //BRect chk1(68,110,220,110);
   //disablecallwaiting = new BCheckBox(chk1,"Disable call waiting","Disable call waiting:", new BMessage(CHK_CALLWAITING), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   //BRect chk2(68,137,220,110);
   //dialoutprefix = new BCheckBox(chk2,"Dial out prefix","Dial out prefix:", new BMessage(CHK_DIALOUTPREFIX), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   
   // Connection MenuField
   BRect mfld1(2,13,180,33);
   BMenuItem *menunew;
   BMenuItem *menudelete;
   
   conmenufield = new BMenu("Click to add");
   conmenufield->AddItem(menunew = new BMenuItem("New...", new BMessage(MENU_NEW)));
   menunew->SetTarget(be_app);
   conmenufield->AddSeparatorItem();
   conmenufield->AddItem(menudelete = new BMenuItem("Delete Current", new BMessage(MENU_DELETE_CURRENT)));
   menudelete->SetEnabled(false);
   menudelete->SetTarget(be_app);
   
   connectionmenufield = new BMenuField(mfld1,"connection_menu","",conmenufield,B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
   
   
   // Displays - <Create a connection profile to continue.> on the main window
   BRect tvCPLocation(40,43,300,10);
   tvConnectionProfile = new BTextView(r, "Connection Profile", tvCPLocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvConnectionProfile->SetText("<Create a connection profile to continue.>");
   
   // Displays - Call waiting may be enabled.
   BRect tvCWLocation(40,113,300,10);
   tvCallWaiting = new BTextView(r, "Call waiting", tvCWLocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvCallWaiting->SetText("Call waiting may be enabled.");
   
   // Displays - No Connection
   BRect tvNCLocation(21,168,300,10);
   tvConnection = new BTextView(r, "No Connection", tvNCLocation, B_FOLLOW_ALL, B_WILL_DRAW);
   tvConnection->SetText("No Connection");
        
   // Outline List View - Fake ones really as we're only using them as an indicator.
   BListItem *conitem;
   BListItem *consubitem;
   BRect lst1(20,44,100,80);
   BListItem *locitem;
   BListItem *locsubitem;
   BRect lst2(20,114,100,170);
     
   connectionlistitem = new BOutlineListView(lst1, "connection_list", B_MULTIPLE_SELECTION_LIST);
   connectionlistitem->AddItem(conitem = new BStringItem(""));
   connectionlistitem->AddUnder(consubitem = new BStringItem(""), conitem);
   connectionlistitem->Collapse(conitem);
   locationlistitem = new BOutlineListView(lst2, "location_list", B_SINGLE_SELECTION_LIST);
   locationlistitem->AddItem(locitem = new BStringItem(""));
   locationlistitem->AddUnder(locsubitem = new BStringItem(""), locitem);
   locationlistitem->Collapse(locitem);
                  
   // Frames
   BRect tf(10,18,302,67);    
   topframe = new BBox(tf,"Connect to:",B_FOLLOW_ALL | B_WILL_DRAW | B_NAVIGABLE_JUMP, B_FANCY_BORDER);
   BRect mf(10,86,302,139);    
   middleframe = new BBox(mf,"From Location:",B_FOLLOW_ALL | B_WILL_DRAW | B_NAVIGABLE_JUMP, B_FANCY_BORDER);
   BRect bf(10,149,302,208);    
   bottomframe = new BBox(bf,"Connection",B_FOLLOW_ALL | B_WILL_DRAW | B_NAVIGABLE_JUMP, B_FANCY_BORDER);
   
   // Set Labels for Frames
   topframe->SetLabel("Connect to:");
   middleframe->SetLabel("From Location:");
   bottomframe->SetLabel("Connection");
           
   // Add our Objects to the Window
  
   AddChild(modembutton);
   AddChild(disconnectbutton);
   AddChild(connectbutton);
   AddChild(connectionlistitem);
   AddChild(locationlistitem);
  
//   AddChild(disablecallwaiting);
//   AddChild(dialoutprefix);
   AddChild(topframe);
   AddChild(middleframe);
   AddChild(bottomframe);
   AddChild(tvConnectionProfile);
   AddChild(tvCallWaiting);
   AddChild(tvConnection);
   //AddChild(connectionmenufield);

     
   // Disable Buttons that need to be
   disconnectbutton->SetEnabled(false);
   connectbutton->SetEnabled(false);
   
   // Set Default Button
   connectbutton->MakeDefault(true);
      
   // Add the Drawing View
   AddChild(aDUNview = new DUNView(r));

}
// ------------------------------------------------------------------------------- //

void DUNView::MouseDown(BPoint bp) {
  //BString *tmp;
   (new BAlert("","Clicked","Okay"))->Go();
}

// DUNWindow::~DUNWindow -- destructor
DUNWindow::~DUNWindow() {
   exit(0);
}
// ------------------------------------------------------------------------------- //

// DUNWindow::QuitRequested -- Post a message to the app to quit
bool DUNWindow::QuitRequested() {
   be_app->PostMessage(B_QUIT_REQUESTED);
   return true;
}
// ------------------------------------------------------------------------------- //

// DUNWindow::MessageReceived -- receives messages
void DUNWindow::MessageReceived (BMessage *message) {
   switch(message->what) {
   	   case BTN_MODEM:
   	   	 (new BAlert("","Modem Button","Okay"))->Go(); 	
   	     break;	
       default:
         BWindow::MessageReceived(message);
         break;
   }
}
// end
