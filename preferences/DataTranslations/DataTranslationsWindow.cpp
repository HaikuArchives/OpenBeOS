/*
 * DataTranslationsWindow.cpp
 * DataTranslations mccall@digitalparadise.co.uk
 *
 */
 
#include <Application.h>
#include <Message.h>
#include <Screen.h>


#include "DataTranslationsMessages.h"
#include "DataTranslationsWindow.h"
#include "DataTranslations.h"

#define DATA_TRANSLATIONS_WINDOW_RIGHT	400
#define DATA_TRANSLATIONS_WINDOW_BOTTOM	300

DataTranslationsWindow::DataTranslationsWindow()
				: BWindow(BRect(0,0,DATA_TRANSLATIONS_WINDOW_RIGHT,DATA_TRANSLATIONS_WINDOW_BOTTOM), "DataTranslations", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	BScreen screen;

	MoveTo(dynamic_cast<DataTranslationsApplication *>(be_app)->WindowCorner());

	// Code to make sure that the window doesn't get drawn off screen...
	if (!(screen.Frame().right >= Frame().right && screen.Frame().bottom >= Frame().bottom))
		MoveTo((screen.Frame().right-Bounds().right)*.5,(screen.Frame().bottom-Bounds().bottom)*.5);

	BuildView();
	AddChild(fView);

	Show();

}

void
DataTranslationsWindow::BuildView()
{
	fView = new DataTranslationsView(Bounds());	
}

bool
DataTranslationsWindow::QuitRequested()
{

	dynamic_cast<DataTranslationsApplication *>(be_app)->SetWindowCorner(BPoint(Frame().left,Frame().top));

	be_app->PostMessage(B_QUIT_REQUESTED);
	
	return(true);
}

void
DataTranslationsWindow::MessageReceived(BMessage *message)
{			
	switch(message->what) {
		default:
			BWindow::MessageReceived(message);
			break;
	}
	
}

DataTranslationsWindow::~DataTranslationsWindow()
{

}