/*

DUNView by Sikosis (beos@gravity24hr.com)

(C) 2002 OpenBeOS under MIT license

*/

#include "app/Application.h"
#include "interface/Window.h"
#include "interface/View.h"
#include <stdio.h>

#include "DUN.h"
#include "DUNWindow.h"
#include "DUNView.h"

// DUNView -- the view so we can put objects like text boxes on it
DUNView::DUNView (BRect frame) : BView (frame, "DUNView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW ) {
	// Set the Background Color
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

void DUNView::Draw (BRect updateRect) {
   // display all the information on the form
   //MovePenTo(BPoint(40,54));
   //DrawString ("<Create a connection profile to continue.>");
   
   //MovePenTo(BPoint(40,124));
   //DrawString ("Call waiting may be enabled.");
         
   MovePenTo(BPoint(22,178));
   DrawString ("No Connection");
   
   MovePenTo(BPoint(249,178));
   DrawString ("00:00:00");

   MovePenTo(BPoint(22,195));
   DrawString ("Local IP address:");

   MovePenTo(BPoint(265,195));
   DrawString ("None");
}
// end
