/*

DUNWindow Header by Sikosis

(C) 2002

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
   void _InitWindow(void);
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
};

#endif
