/*
 * MouseView.cpp
 * Open BeOS version alpha 1 by Andrew Edward McCall mccall@digitalparadise.co.uk
 *
 */
 
#include <Box.h>
#include <Button.h>
#include <Bitmap.h>
#include <Slider.h>
#include <TextControl.h>
#include <InterfaceDefs.h>
#include <TranslationUtils.h>

#include "MouseView.h"
#include "MouseMessages.h"

MouseView::MouseView(BRect rect)
			: BView(rect, "mouse_view", B_FOLLOW_ALL, B_WILL_DRAW)
{	
	BRect 			aRect;
	BBox			*lineBox;
	BButton 		*aButton;
	BTextControl 	*aTextControl;
	
	// Set the color to grey...
	SetViewColor(ui_color( B_PANEL_BACKGROUND_COLOR ));
	
	//Lets load the icons we will need
	fDoubleClickBitmap = BTranslationUtils::GetBitmap("double_click_bmap");
	fSpeedBitmap = BTranslationUtils::GetBitmap("speed_bmap");
	fAccelerationBitmap = BTranslationUtils::GetBitmap("acceleration_bmap");

	//Add the "Default" button..	
	aRect.Set(10,259,85,279);
	aButton = new BButton(aRect,"mouse_defaults","Defaults", new BMessage(BUTTON_DEFAULTS));
	AddChild(aButton);
	// Add the "Revert" button...
	aRect.Set(92,259,167,279);
	fRevertButton = new BButton(aRect,"mouse_revert","Revert", new BMessage(BUTTON_REVERT));
	fRevertButton->SetEnabled(false);
	AddChild(fRevertButton);
	
	// Create the box for the sliders...
	aRect=Bounds();
	aRect.left=aRect.left+11;
	aRect.top=aRect.top+11;
	aRect.right=aRect.right-11;
	aRect.bottom=aRect.bottom-44;
	fBox = new BBox(aRect,"mouse_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	
	// Before building the sliders, get the current values of the double click speed
	// mouse speed and acceleration.
	if (get_click_speed(&fDoubleClickSpeed)!=B_OK)
			be_app->PostMessage(ERROR_DETECTED);
	if (get_mouse_speed(&fMouseSpeed)!=B_OK)
	    	be_app->PostMessage(ERROR_DETECTED);
	
	// Create the "Double-click speed slider...
	aRect.Set(168,10,328,50);
	fClickSlider = new BSlider(aRect,"key_repeat_rate","Double-click speed", new BMessage(SLIDER_DOUBLE_CLICK_SPEED),0,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	fClickSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fClickSlider->SetHashMarkCount(5);
	fClickSlider->SetLimitLabels("Slow","Fast");
	fClickSlider->SetValue(fDoubleClickSpeed);
	fBox->AddChild(fClickSlider);
	// Create the "Delay until key repeat" slider...
	aRect.Set(168,65,328,115);
	fSpeedSlider = new BSlider(aRect,"delay_until_key_repeat","Mouse Speed", new BMessage(SLIDER_MOUSE_SPEED),250000,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	fSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fSpeedSlider->SetHashMarkCount(7);
	fSpeedSlider->SetValue(fMouseSpeed);
	fSpeedSlider->SetLimitLabels("Slow","Fast");
	fBox->AddChild(fSpeedSlider);
	
	// Create the line above the test area
	aRect.Set(10,199,150,200);
	lineBox = new BBox(aRect,"line_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	fBox->AddChild(lineBox);
	// Create the line above the test area
	aRect.Set(170,199,362,200);
	lineBox = new BBox(aRect,"line_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	fBox->AddChild(lineBox);
	// Create the line above the test area
	aRect.Set(160,10,161,230);
	lineBox = new BBox(aRect,"line_box",B_FOLLOW_LEFT,B_WILL_DRAW,B_FANCY_BORDER);
	fBox->AddChild(lineBox);
	// Create the "Double-click test area" text box...
	aRect=fBox->Bounds();
	aRect.left=aRect.left+10;
	aRect.right=aRect.right-230;
	aRect.top=aRect.bottom-30;
	aTextControl = new BTextControl(aRect,"double_click_test_area",NULL,"Double-click test area", new BMessage('DCta'),B_FOLLOW_LEFT,B_WILL_DRAW);
	aTextControl->SetAlignment(B_ALIGN_LEFT,B_ALIGN_CENTER);
	fBox->AddChild(aTextControl);	
	AddChild(fBox);
}

void
MouseView::Draw(BRect updateFrame)
{
	fBox->DrawBitmap(fDoubleClickBitmap,BPoint(341,20));
	fBox->DrawBitmap(fSpeedBitmap,BPoint(331,80));	
	fBox->DrawBitmap(fAccelerationBitmap,BPoint(331,115));	
}