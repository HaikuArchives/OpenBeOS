/*
	
	KeyboardView.cpp
	
*/

#ifndef KEYBOARD_VIEW_H
#include "KeyboardView.h"
#endif
#ifndef _BOX_H
#include "Box.h"
#endif
#ifndef _BUTTON_H
#include "Button.h"
#endif
#ifndef _SLIDER_H
#include "Slider.h"
#endif
#ifndef _TEXTCONTROL_H
#include "TextControl.h"
#endif
#ifndef KEYBOARD_MESSAGES_H
#include "KeyboardMessages.h"
#endif
#include <stdio.h>

KeyboardView::KeyboardView(BRect rect)
	   	   : BView(rect, "keyboard_view", B_FOLLOW_ALL, B_WILL_DRAW)
{
	BRect 		aRect;
	BBox  		*aBox;
	BButton 	*aButton;
	BTextControl *aTextControl;
	
	// Set the color to grey...
	SetViewColor(216,216,216,0);
	//Add the "Default" button..	
	aRect.Set(10,187,85,207);
	aButton = new BButton(aRect,"keyboard_defaults","Defaults", new BMessage(BUTTON_DEFAULTS));
	AddChild(aButton);
	// Add the "Revert" button...
	aRect.Set(92,187,167,207);
	aButton = new BButton(aRect,"keyboard_revert","Revert", new BMessage(BUTTON_REVERT));
	aButton->SetEnabled(false);
	AddChild(aButton);
	// Create the box for the sliders...
	aRect=Bounds();
	aRect.left=aRect.left+11;
	aRect.top=aRect.top+12;
	aRect.right=aRect.right-11;
	aRect.bottom=aRect.bottom-44;
	aBox = new BBox(aRect,"keyboard_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	// Before building the sliders, get the current values of the key repeat rate
	// and the delay rate.
	if (get_key_repeat_rate(&rrate)!=B_OK)
		{
			be_app->PostMessage(ERROR_DETECTED);
		}
	if (get_key_repeat_delay(&drate)!=B_OK) 
	  	{
	    	be_app->PostMessage(ERROR_DETECTED);
	  	}
	// Create the "Key repeat rate" slider...
	aRect.Set(10,10,200,50);
	rateSlider = new BSlider(aRect,"key_repeat_rate","Key repeat rate", new BMessage(SLIDER_REPEAT_RATE),20,300,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	rateSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	rateSlider->SetHashMarkCount(5);
	rateSlider->SetLimitLabels("Slow","Fast");
	rateSlider->SetValue(rrate);
	aBox->AddChild(rateSlider);
	// Create the "Delay until key repeat" slider...
	aRect.Set(10,65,200,115);
	delaySlider = new BSlider(aRect,"delay_until_key_repeat","Delay until key repeat", new BMessage(SLIDER_DELAY_RATE),250000,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	delaySlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	delaySlider->SetHashMarkCount(4);
	delaySlider->SetValue(drate);
	delaySlider->SetKeyIncrementValue(250000);
	delaySlider->SetLimitLabels("Short","Long");
	aBox->AddChild(delaySlider);
	// Create the "Typing test area" text box...
	aRect=Bounds();
	aRect.left=aRect.left+10;
	aRect.top=135;
	aRect.right=aRect.right-34;
	aRect.bottom=aRect.bottom-11;
	aTextControl = new BTextControl(aRect,"typing_test_area",NULL,"Typing test area", new BMessage('TTEA'),B_FOLLOW_LEFT,B_WILL_DRAW);
	aTextControl->SetAlignment(B_ALIGN_LEFT,B_ALIGN_CENTER);
	aBox->AddChild(aTextControl);	
	AddChild(aBox);
	
}