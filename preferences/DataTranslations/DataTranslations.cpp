/*
 * DataTranslations.cpp
 * DataTranslations mccall@digitalparadise.co.uk
 *
 */

#include <Alert.h>
#include <Screen.h>

#include "DataTranslations.h"
#include "DataTranslationsWindow.h"
#include "DataTranslationsSettings.h"
#include "DataTranslationsMessages.h"

const char DataTranslationsApplication::kDataTranslationsApplicationSig[] = "application/x-vnd.OpenBeOS-prefs-translations";

int main(int, char**)
{
	DataTranslationsApplication	myApplication;

	myApplication.Run();

	return(0);
}

DataTranslationsApplication::DataTranslationsApplication()
					:BApplication(kDataTranslationsApplicationSig)
{

	DataTranslationsWindow		*window;
	
	fSettings = new DataTranslationsSettings();
		
	window = new DataTranslationsWindow();

}

void
DataTranslationsApplication::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case ERROR_DETECTED:
			{
				BAlert *errorAlert = new BAlert("Error", "Something has gone wrong!","OK",NULL,NULL,B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				errorAlert->Go();
				be_app->PostMessage(B_QUIT_REQUESTED);
			}
			break;			
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

void
DataTranslationsApplication::SetWindowCorner(BPoint corner)
{
	fSettings->SetWindowCorner(corner);
}

void
DataTranslationsApplication::AboutRequested(void)
{
	(new BAlert("about", "...by Andrew Edward McCall", "Dig Deal"))->Go();
}

DataTranslationsApplication::~DataTranslationsApplication()
{
	delete fSettings;
}