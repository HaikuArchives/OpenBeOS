#ifndef CACHE_VIEW_H

	#include "CacheView.h"

#endif

CacheView::CacheView(BRect rect, int minVal, int maxVal, int currVal)
	   	   : BView(rect, "CacheView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	
	float x;
	float y;
	BRect viewSize = Bounds();
	char sliderMinLabel[10];
	char sliderMaxLabel[10];
	
	SetViewColor(216, 216, 216, 0);
	
	rgb_color fillColor;
	fillColor.red = 255;
	fillColor.blue = 0;
	fillColor.green = 0;
	
	viewSize.InsetBy(15, 10);
	
	screenFCS = new BSlider(*(new BRect(viewSize.left, viewSize.top, viewSize.right, viewSize.top + 25.0)),
							"screenFontCache",
							"Screen font cache size: ",
							new BMessage(SCREEN_FCS_UPDATE_MSG),
							minVal,
							maxVal,
							B_TRIANGLE_THUMB
						   );
	screenFCS->SetModificationMessage(new BMessage(SCREEN_FCS_MODIFICATION_MSG));
	
	sprintf(sliderMinLabel, "%d kB", minVal);
	sprintf(sliderMaxLabel, "%d kB", maxVal);
	screenFCS->SetLimitLabels(sliderMinLabel, sliderMaxLabel);
	screenFCS->UseFillColor(TRUE, &fillColor);
	screenFCS->SetValue(currVal);
	
	viewSize.top = viewSize.top + 65.0;
	
	printFCS = new BSlider(*(new BRect(viewSize.left, viewSize.top, viewSize.right, viewSize.top + 25.0)),
							"printFontCache",
							"Printing font cache size: ",
							new BMessage(PRINT_FCS_UPDATE_MSG),
							minVal,
							maxVal,
							B_TRIANGLE_THUMB
						   );
	printFCS->SetModificationMessage(new BMessage(PRINT_FCS_MODIFICATION_MSG));
	
	printFCS->SetLimitLabels(sliderMinLabel, sliderMaxLabel);
	printFCS->UseFillColor(TRUE, &fillColor);
	printFCS->SetValue(currVal);
	
	viewSize.top = viewSize.top + 70.0;
	
	BButton *saveCache = new BButton(*(new BRect(viewSize.left, viewSize.top, 100.0, 20.0)),
							   "saveCache",
							   "Save Cache",
							   NULL,
							   B_FOLLOW_LEFT,
							   B_WILL_DRAW
							  );
	
	AddChild(screenFCS);
	AddChild(printFCS);
	AddChild(saveCache);  
	
}
