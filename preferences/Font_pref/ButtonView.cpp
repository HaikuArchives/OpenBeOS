#ifndef BUTTON_VIEW_H

	#include "ButtonView.h"

#endif

ButtonView::ButtonView(BRect rect)
	   	   : BView(rect, "ButtonView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	
	float x;
	float y;
	BRect viewSize = Bounds();
	BButton *rescanButton;
	BButton *defaultsButton;
	
	x = viewSize.Width() / 37;
	y = viewSize.Height() / 6;
	
	SetViewColor(216,216,216,0);
	
	rescanButton = new BButton(*(new BRect(x, y, (9.0 * x), (4.0 * y))),
							   "rescanButton",
							   "Rescan",
							   new BMessage(RESCAN_FONTS_MSG),
							   B_FOLLOW_LEFT,
							   B_WILL_DRAW
							  );
	defaultsButton = new BButton(*(new BRect((11.0 * x), y, (19.0 * x), (4.0 * y))),
							   "defaultsButton",
							   "Defaults",
							   new BMessage(RESET_FONTS_MSG),
							   B_FOLLOW_LEFT,
							   B_WILL_DRAW
							  );
	revertButton = new BButton(*(new BRect((20.0 * x), y, (28.0 * x), (4.0 * y))),
							   "revertButton",
							   "Revert",
							   new BMessage(REVERT_MSG),
							   B_FOLLOW_LEFT,
							   B_WILL_DRAW
							  );
							  
	revertButton->SetEnabled(false);
	
	AddChild(rescanButton);
	AddChild(defaultsButton);
	AddChild(revertButton);
	
}

