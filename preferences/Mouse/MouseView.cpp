/*
	
	MouseView.cpp
	
*/

#ifndef KEYBOARD_VIEW_H
#include "MouseView.h"
#endif
#ifndef _BOX_H
#include <Box.h>
#endif
#ifndef _BUTTON_H
#include <Button.h>
#endif
#ifndef _SLIDER_H
#include <Slider.h>
#endif
#ifndef _TEXTCONTROL_H
#include <TextControl.h>
#endif
#ifndef MOUSE_MESSAGES_H
#include "MouseMessages.h"
#endif
#include <InterfaceDefs.h>
#include <stdio.h>

MouseView::MouseView(BRect rect)
	   	   : BView(rect, "mouse_view", B_FOLLOW_ALL, B_WILL_DRAW)
{	
	BRect 			aRect;
	BButton 		*aButton;
	BBox			*aBox;
	BTextControl 	*aTextControl;
	
	// Set the color to grey...
	SetViewColor(ui_color( B_PANEL_BACKGROUND_COLOR ));
	//Add the "Default" button..	
	aRect.Set(10,259,85,279);
	aButton = new BButton(aRect,"mouse_defaults","Defaults", new BMessage(BUTTON_DEFAULTS));
	AddChild(aButton);
	// Add the "Revert" button...
	aRect.Set(92,259,167,279);
	revertButton = new BButton(aRect,"mouse_revert","Revert", new BMessage(BUTTON_REVERT));
	revertButton->SetEnabled(false);
	AddChild(revertButton);
	
		// Create the box for the sliders...
	aRect=Bounds();
	aRect.left=aRect.left+11;
	aRect.top=aRect.top+11;
	aRect.right=aRect.right-11;
	aRect.bottom=aRect.bottom-44;
	aBox = new BBox(aRect,"mouse_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	
	// Before building the sliders, get the current values of the double click speed
	// mouse speed and acceleration.
	if (get_click_speed(&dcspeed)!=B_OK)
		{
			be_app->PostMessage(ERROR_DETECTED);
		}
	if (get_mouse_speed(&mspeed)!=B_OK) 
	  	{
	    	be_app->PostMessage(ERROR_DETECTED);
	  	}
	// Create the "Double-click speed slider...
	aRect.Set(168,10,328,50);
	clickSlider = new BSlider(aRect,"key_repeat_rate","Double-click speed", new BMessage(SLIDER_DOUBLE_CLICK_SPEED),0,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	clickSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	clickSlider->SetHashMarkCount(5);
	clickSlider->SetLimitLabels("Slow","Fast");
	clickSlider->SetValue(dcspeed);
	aBox->AddChild(clickSlider);
	// Create the "Delay until key repeat" slider...
	aRect.Set(168,65,328,115);
	speedSlider = new BSlider(aRect,"delay_until_key_repeat","Mouse Speed", new BMessage(SLIDER_MOUSE_SPEED),250000,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	speedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	speedSlider->SetHashMarkCount(7);
	speedSlider->SetValue(mspeed);
	speedSlider->SetLimitLabels("Slow","Fast");
	aBox->AddChild(speedSlider);
	// Create the "Double-click test area" text box...
	aRect=Bounds();
	aRect.left=aRect.left+10;
	aRect.top=135;
	aRect.right=aRect.right-34;
	aRect.bottom=aRect.bottom-11;
	aTextControl = new BTextControl(aRect,"double_click_test_area",NULL,"Double-click test area", new BMessage('DCta'),B_FOLLOW_LEFT,B_WILL_DRAW);
	aTextControl->SetAlignment(B_ALIGN_LEFT,B_ALIGN_CENTER);
	aBox->AddChild(aTextControl);	
	AddChild(aBox);

	
	
}