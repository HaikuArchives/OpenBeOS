#include "DrawMonitors.h"
#include <Rect.h>
#include <Point.h>

/*
class previewView : public BView
{ 
	virtual void Draw(); 
	public:	
	BView *previewArea;
}

class mouseAreaView : public BView
{
	virtual void Draw(); 
}
*/

inline BPoint scaleDirect(float x, float y,BRect area)
{
	return BPoint(area.Width()*x+area.left,area.Height()*y+area.top);
}

inline BRect scaleDirect (float x1,float x2,float y1,float y2,BRect area)
{
	return BRect(area.Width()*x1+area.left,area.Height()*y1+area.top, area.Width()*x2+area.left,area.Height()*y2+area.top);
}

float positionalX[]= {0,.1,.25,.3,.7,.75,.9,1.0};
float positionalY[]= {0,.1,.7,.8,.9,1.0};

inline BPoint scale(int x, int y,BRect area) { return scaleDirect(positionalX[x],positionalY[y],area); }
inline BRect scale(int x1, int x2, int y1, int y2,BRect area) { return scaleDirect(positionalX[x1],positionalX[x2],positionalY[y1],positionalY[y2],area); }

rgb_color black = {0,0,0,0};
rgb_color darkGrey = {150,150,150,0};
rgb_color grey = {200,200,200,0};
rgb_color lightBlue = {200,200,255,0};
rgb_color lightGreen = {255,200,200,0};

void mouseAreaView::Draw(void)
{
	// Top of monitor
	SetHighColor(grey);
	FillRoundRect(scale(0,7,0,3,area),1,1);
	SetHighColor(black);
	StrokeRoundRect(scale(0,7,0,3,area),1,1);
	SetHighColor(lightBlue);
	FillRect(scale(1,6,1,2,area));
	// Base of monitor
	SetHighColor(grey);
	FillTriangle(scale(1,5,area),scale(3,4,area),scale(3,5,area));
	FillTriangle(scale(4,5,area),scale(4,4,area),scale(6,5,area));
	FillRect(scale(3,4,4,5,area));
	FillRect(scale(3,4,3,4,area));
	SetHighColor(black);
	StrokeRect(scale(3,4,3,4,area));
	StrokeLine(scale(2,4,area),scale(1,5,area));
	StrokeLine(scale(6,5,area),scale(1,5,area));
	StrokeLine(scale(6,5,area),scale(5,4,area));
}

float sampleX[]= {0,.025,.25,.6,.625,.7,.725,.75,.975,1.0};
float sampleY[]= {0,.05,.8,.9,.933,.966,1.0};
inline BPoint scale2(int x, int y,BRect area) { return scaleDirect(sampleX[x],sampleY[y],area); }
inline BRect scale2(int x1, int x2, int y1, int y2,BRect area) { return scaleDirect(sampleX[x1],sampleX[x2],sampleY[y1],sampleY[y2],area); }
BView *drawSampleMonitor(BView *view, BRect area)
void previewView::Draw(void)
{
	BView *newView;
	SetHighColor(grey);
	FillRoundRect(scale2(0,9,0,3,area),1,1);
	SetHighColor(black);
	StrokeRoundRect(scale2(0,9,0,3,area),1,1);
//	AddChild(newView=new BView (scale2(1,8,1,2,area),"sampleScreen",B_FOLLOW_NONE,0));
//	newView->SetViewColor(lightBlue);
	FillRoundRect(scale2(1,8,1,2,area),1,1);
	SetHighColor(grey);
	FillRoundRect(scale2(2,7,3,6,area),1,1);
	SetHighColor(black);
	StrokeLine(scale2(2,3,area),scale2(2,6,area));
	StrokeLine(scale2(2,6,area),scale2(7,6,area));
	StrokeLine(scale2(7,6,area),scale2(7,3,area));
	SetHighColor(lightGreen);
	FillRect(scale2(3,4,4,5,area));
	SetHighColor(darkGrey);
	FillRect(scale2(5,6,4,5,area));
}
