/*
 * MouseView.cpp
 * Open BeOS version alpha 1 by Andrew Edward McCall mccall@digitalparadise.co.uk
 *
 */
 
#include <Box.h>
#include <Menu.h>
#include <Button.h>
#include <Bitmap.h>
#include <Slider.h>
#include <TextControl.h>
#include <InterfaceDefs.h>
#include <TranslationUtils.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <MenuField.h>

#include "MouseView.h"
#include "MouseMessages.h"

MouseView::MouseView(BRect rect)
			: BView(rect, "mouse_view", B_FOLLOW_ALL, B_WILL_DRAW)
{	
	BRect 			aRect;
	BButton 		*aButton;
	BTextControl 	*aTextControl;
	
	// Set the color to grey...
	SetViewColor(ui_color( B_PANEL_BACKGROUND_COLOR ));
	
	// Lets load the icons we will need
	fDoubleClickBitmap = BTranslationUtils::GetBitmap("double_click_bmap");
	fSpeedBitmap = BTranslationUtils::GetBitmap("speed_bmap");
	fAccelerationBitmap = BTranslationUtils::GetBitmap("acceleration_bmap");
	fMouseBodyBitmap = BTranslationUtils::GetBitmap("mouse_body_bmap");
	fOneButtonNormal = BTranslationUtils::GetBitmap("one_button_normal_bmap");

	// Add the "Default" button..	
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
	
	// Add the "Mouse Type" pop up menu
	fMouseTypeMenu = new BPopUpMenu("Mouse Type Menu");
	fMouseTypeMenu->AddItem(new BMenuItem("1-Button",new BMessage(BUTTON_REVERT)));
	fMouseTypeMenu->AddItem(new BMenuItem("2-Button",new BMessage(BUTTON_REVERT)));
	fMouseTypeMenu->AddItem(new BMenuItem("3-Button",new BMessage(BUTTON_REVERT)));
	fMouseTypeMenu->ItemAt(2)->SetMarked(true);		

	aRect.Set(18,10,208,20);
	fMouseTypeSelector = new BMenuField(aRect, "Mouse Type", "Mouse Type", fMouseTypeMenu);
	fMouseTypeSelector->SetDivider(fMouseTypeSelector->Divider() - 30);
	fBox->AddChild(fMouseTypeSelector);
	
	// Add the "Mouse Type" pop up menu
	fFocusMenu = new BPopUpMenu("Focus Follows Mouse Menu");
	fFocusMenu->AddItem(new BMenuItem("Disabled",new BMessage(BUTTON_REVERT)));
	fFocusMenu->AddItem(new BMenuItem("Enabled",new BMessage(BUTTON_REVERT)));
	fFocusMenu->AddItem(new BMenuItem("Warping",new BMessage(BUTTON_REVERT)));
	fFocusMenu->AddItem(new BMenuItem("Instant-Warping",new BMessage(BUTTON_REVERT)));
	fFocusMenu->ItemAt(0)->SetMarked(true);		

	aRect.Set(168,207,440,200);
	fFocusTypeSelector = new BMenuField(aRect, "Focus follows mouse", "Focus follows mouse:", fFocusMenu);
	fFocusTypeSelector->SetDivider(fFocusTypeSelector->Divider() - 30);
	fBox->AddChild(fFocusTypeSelector);
	
	// Before building the sliders, get the current values of the double click speed
	// mouse speed and acceleration.
	if (get_click_speed(&fDoubleClickSpeed)!=B_OK)
			be_app->PostMessage(ERROR_DETECTED);
	if (get_mouse_speed(&fMouseSpeed)!=B_OK)
	    	be_app->PostMessage(ERROR_DETECTED);
	
	// Create the "Double-click speed slider...
	aRect.Set(168,10,328,50);
	fClickSlider = new BSlider(aRect,"double_click_speed","Double-click speed", new BMessage(SLIDER_DOUBLE_CLICK_SPEED),0,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	fClickSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fClickSlider->SetHashMarkCount(5);
	fClickSlider->SetLimitLabels("Slow","Fast");
	fClickSlider->SetValue(fDoubleClickSpeed);
	fBox->AddChild(fClickSlider);
	
	// Create the "Mouse Speed" slider...
	aRect.Set(168,75,328,125);
	fSpeedSlider = new BSlider(aRect,"mouse_speed","Mouse Speed", new BMessage(SLIDER_MOUSE_SPEED),250000,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	fSpeedSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fSpeedSlider->SetHashMarkCount(7);
	fSpeedSlider->SetValue(fMouseSpeed);
	fSpeedSlider->SetLimitLabels("Slow","Fast");
	fBox->AddChild(fSpeedSlider);

	// Create the "Mouse Acceleration" slider...
	aRect.Set(168,140,328,190);
	fAccelerationSlider = new BSlider(aRect,"mouse_acceleration","Mouse Acceleration", new BMessage(SLIDER_MOUSE_SPEED),250000,1000000,B_BLOCK_THUMB,B_FOLLOW_LEFT,B_WILL_DRAW);
	fAccelerationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fAccelerationSlider->SetHashMarkCount(5);
	//fAccelerationSlider->SetValue(fMouseSpeed);
	fAccelerationSlider->SetLimitLabels("Slow","Fast");
	fBox->AddChild(fAccelerationSlider);
	
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
	fBox->SetHighColor(120,120,120);
	fBox->SetLowColor(255,255,255);
	// Line above the test area
	fBox->StrokeLine(BPoint(10,199),BPoint(150,199),B_SOLID_HIGH);
	fBox->StrokeLine(BPoint(11,200),BPoint(150,200),B_SOLID_LOW);
	// Line above focus follows mouse
	fBox->StrokeLine(BPoint(170,199),BPoint(362,199),B_SOLID_HIGH);
	fBox->StrokeLine(BPoint(171,200),BPoint(362,200),B_SOLID_LOW);
	// Line in the middle
	fBox->StrokeLine(BPoint(160,10),BPoint(160,230),B_SOLID_HIGH);
	fBox->StrokeLine(BPoint(161,11),BPoint(161,230),B_SOLID_LOW);
	//Draw the icons
	fBox->DrawBitmap(fDoubleClickBitmap,BPoint(341,20));
	fBox->DrawBitmap(fSpeedBitmap,BPoint(331,90));	
	fBox->DrawBitmap(fAccelerationBitmap,BPoint(331,155));
	fBox->DrawBitmap(fMouseBodyBitmap,BPoint(50,70));
	fBox->DrawBitmap(fOneButtonNormal,BPoint(50,38));
}