/*
	
	DataTranslationsView.cpp
	
*/
#include <InterfaceDefs.h>

#include "DataTranslationsView.h"
#include "DataTranslationsMessages.h"


DataTranslationsView::DataTranslationsView(BRect rect)
	   	   : BBox(rect, "data_translations_view",
					B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER)
{

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

}