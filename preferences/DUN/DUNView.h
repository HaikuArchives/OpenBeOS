/*

DUNView by Sikosis (beos@gravity24hr.com)

(C) 2002 OpenBeOS under MIT license

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
