#ifndef DATA_TRANSLATIONS_H
#define DATA_TRANSLATIONS_H

#include <Application.h>

#include "DataTranslationsWindow.h"
#include "DataTranslationsSettings.h"

class DataTranslationsApplication : public BApplication 
{
public:
	DataTranslationsApplication();
	virtual ~DataTranslationsApplication();
	
	void MessageReceived(BMessage *message);
	BPoint WindowCorner() const {return fSettings->WindowCorner(); }
	void SetWindowCorner(BPoint corner);

	void AboutRequested(void);
	
private:
	
	static const char kDataTranslationsApplicationSig[];

	DataTranslationsSettings		*fSettings;

};

#endif