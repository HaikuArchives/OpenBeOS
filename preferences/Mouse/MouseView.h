/*
	
	MouseView.h
	
*/

#ifndef MOUSE_VIEW_H
#define MOUSE_VIEW_H


#ifndef _VIEW_H
#include <View.h>
#endif
#ifndef _SLIDER_H
#include <Slider.h>
#endif
#ifndef _SUPPORTDEFS_H
#include <SupportDefs.h>
#endif
#ifndef _INTERFACEDEFS_H
#include <InterfaceDefs.h>
#endif
#ifndef _APPLICATION_H
#include <Application.h>
#endif

class MouseView : public BView 
{
public:
		MouseView(BRect frame);
		virtual void Draw(BRect updateFrame);
private:
		BButton		*revertButton;
		bigtime_t	dcspeed;
		int32		mspeed;
		BSlider		*clickSlider;
		BSlider		*speedSlider;
		BBitmap 	*double_click_bitmap;
		BBitmap 	*speed_bitmap;
		BBitmap 	*acceleration_bitmap;
		BBox		*aBox;
};

#endif //MOUSE_VIEW_H
