/*

DUNWindow Header by Sikosis (beos@gravity24hr.com)

(C) 2002 OpenBeOS under MIT license

*/

#ifndef __DUNWINDOW_H__
#define __DUNWINDOW_H__

class DUNView; 

class DUNWindow : public BWindow {
public:
   DUNWindow(BRect frame);
   ~DUNWindow();
   virtual bool QuitRequested();
   virtual void MessageReceived(BMessage *message);
private:
   void InitWindow(void);
   DUNView* aDUNview;
   
   //BMenuBar *menubar;
   BBox *topframe;
   BBox *middleframe;
   BBox *bottomframe;
   
   BButton *modembutton;
   BButton *disconnectbutton;
   BButton *connectbutton;
   
   BCheckBox *disablecallwaiting;
   BCheckBox *dialoutprefix;
   
   BOutlineListView *connectionlistitem;
   BOutlineListView *locationlistitem;
   
   BMenuField *connectionmenufield;
   BMenu *conmenufield;
   
   BTextView *tvConnectionProfile;
   BTextView *tvCallWaiting;
   BTextView *tvConnection;
};

#endif
