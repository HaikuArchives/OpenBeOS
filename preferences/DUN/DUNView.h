/*

DUNView by Sikosis

(C) 2002

*/

#ifndef __DUNVIEW_H__
#define __DUNVIEW_H__

class DUNView : public BView {
public:
   DUNView(BRect frame);
   virtual void Draw (BRect updateRect);
   void MouseDown(BPoint bp);
};

#endif
