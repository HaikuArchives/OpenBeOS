/*
	
	DataTranslationsView.h
	
*/

#ifndef DATA_TRANSLATIONS_VIEW_H
#define DATA_TRANSLATIONS_VIEW_H


#include <Box.h>
#include <Slider.h>
#include <SupportDefs.h>
#include <InterfaceDefs.h>
#include <Application.h>

class DataTranslationsView : public BBox
{
public:
		typedef BBox	inherited;

		DataTranslationsView(BRect frame);

private:
		BBox			*fBox;

};

#endif
