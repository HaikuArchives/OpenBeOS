#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#include <Window.h>

#include "DataTranslationsSettings.h"
#include "DataTranslationsView.h"

class DataTranslationsWindow : public BWindow 
{
public:
	DataTranslationsWindow();
	~DataTranslationsWindow();
	
	bool QuitRequested();
	void MessageReceived(BMessage *message);
	
private:
	void BuildView();

	DataTranslationsView	*fView;
};

#endif
