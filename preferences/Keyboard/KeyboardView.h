/*
	
	KeyboardView.h
	
*/

#ifndef KEYBOARD_VIEW_H
#define KEYBOARD_VIEW_H


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

class KeyboardView : public BView 
{
public:
		KeyboardView(BRect frame);
		BSlider		*rateSlider;
		BSlider		*delaySlider;
private:
        int32            rrate;
		bigtime_t        drate; 
};

#endif //KEYBOARD_VIEW_H
