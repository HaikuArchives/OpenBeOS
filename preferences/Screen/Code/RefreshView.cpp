#include <View.h>

#include "RefreshView.h"

RefreshView::RefreshView(BRect rect, char *name)
	: BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW)
{

}

void RefreshView::AttachedToWindow()
{
	rgb_color greyColor = {216, 216, 216, 255};
	SetViewColor(greyColor);
}

void RefreshView::Draw(BRect updateRect)
{
	rgb_color whiteColor = {255, 255, 255, 255};
	rgb_color darkColor = {128, 128, 128, 255};
	rgb_color blackColor = {0, 0, 0, 255};
	
	SetHighColor(whiteColor);
	
	StrokeLine(BPoint(Bounds().left, Bounds().top), BPoint(Bounds().right, Bounds().top));
	StrokeLine(BPoint(Bounds().left, Bounds().top), BPoint(Bounds().left, Bounds().bottom));
	
	SetHighColor(darkColor);
	
	StrokeLine(BPoint(Bounds().left, Bounds().bottom), BPoint(Bounds().right, Bounds().bottom));
	StrokeLine(BPoint(Bounds().right, Bounds().bottom), BPoint(Bounds().right, Bounds().top));
	
	SetHighColor(blackColor);
	
	DrawString("Type or use the left and right arrow keys.", BPoint(10.0, 23.0));
}
