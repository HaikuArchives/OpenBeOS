#ifndef CACHE_VIEW_H

	#include "CacheView.h"

#endif

CacheView::CacheView(BRect rect, int minVal, int maxVal, int printCurrVal, int screenCurrVal)
	   	   : BView(rect, "CacheView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	
	float x;
	float y;
	BRect viewSize = Bounds();
	char sliderMinLabel[10];
	char sliderMaxLabel[10];
	char msg[100];
	
	SetViewColor(216, 216, 216, 0);
	
	rgb_color fillColor;
	fillColor.red = 255;
	fillColor.blue = 0;
	fillColor.green = 0;
	
	viewSize.InsetBy(15, 10);
	
	sprintf(msg, "Screen font cache size : %d kB", screenCurrVal);
	
	screenFCS = new BSlider(*(new BRect(viewSize.left, viewSize.top, viewSize.right, viewSize.top + 25.0)),
							"screenFontCache",
							msg,
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
	screenFCS->SetValue(screenCurrVal);
	
	viewSize.top = viewSize.top + 65.0;
	
	sprintf(msg, "Printing font cache size : %d kB", printCurrVal);
	
	printFCS = new BSlider(*(new BRect(viewSize.left, viewSize.top, viewSize.right, viewSize.top + 25.0)),
							"printFontCache",
							msg,
							new BMessage(PRINT_FCS_UPDATE_MSG),
							minVal,
							maxVal,
							B_TRIANGLE_THUMB
						   );
	printFCS->SetModificationMessage(new BMessage(PRINT_FCS_MODIFICATION_MSG));
	
	printFCS->SetLimitLabels(sliderMinLabel, sliderMaxLabel);
	printFCS->UseFillColor(TRUE, &fillColor);
	printFCS->SetValue(printCurrVal);
	
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

void CacheView::updatePrintFCS(const char* txt){

	printFCS->SetLabel(txt);

}//updatePrintFCS

void CacheView::updateScreenFCS(const char* txt){

	screenFCS->SetLabel(txt);

}//updatePrintFCS

int CacheView::getPrintFCSValue(){

	return int(printFCS->Value());

}//getPrintValue

int CacheView::getScreenFCSValue(){

	return int(screenFCS->Value());

}//getPrintValue
