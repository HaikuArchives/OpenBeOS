#ifndef MOUSE_VIEW_H
#define MOUSE_VIEW_H

#include <View.h>
#include <Slider.h>
#include <SupportDefs.h>
#include <Application.h>
#include <InterfaceDefs.h>

class MouseView : public BView 
{
public:
		MouseView(BRect frame);
		virtual void Draw(BRect updateframe);
private:
		BButton		*fRevertButton;
		bigtime_t	fDoubleClickSpeed;
		int32		fMouseSpeed;
		BSlider		*fClickSlider;
		BSlider		*fSpeedSlider;
		BBitmap 	*fDoubleClickBitmap;
		BBitmap 	*fSpeedBitmap;
		BBitmap 	*fAccelerationBitmap;
		BBox		*fBox;
};

#endif
