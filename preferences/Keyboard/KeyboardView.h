/*
	
	KeyboardView.h
	
*/

#ifndef KEYBOARD_VIEW_H
#define KEYBOARD_VIEW_H


#ifndef _BOX_H
#include <Box.h>
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

class KeyboardView : public BBox
{
public:
		typedef BBox	inherited;

		KeyboardView(BRect frame);
		virtual void	Draw(BRect frame);
		int32	GetRepeatRate() const { return rateSlider->Value(); }
		bigtime_t GetDelayRate() const { return delaySlider->Value(); }
		void	SetRepeatRate(int32);
		void	SetDelayRate(bigtime_t);
		void	SetRevertButton(bool);
private:
        int32            rrate;
		bigtime_t        drate;
		BSlider			*rateSlider;
		BSlider			*delaySlider;
		BButton			*revertButton;
		BBitmap 		*icon_bitmap;
		BBitmap 		*clock_bitmap;
		BBox  			*aBox;

};

#endif //KEYBOARD_VIEW_H
