#ifndef SCREEN_SAVER_H
#define SCREEN_SAVER_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif
#include "SSAwindow.h"

enum screenSaverWhat {GOBLANK=48, UNBLANK=49, RESET=50, NEWDURATION=51, SETNEWSAVER=52};
enum SSstates {STOP,EXIT,DRAW,DIRECTDRAW};

class ScreenSaverApp : public BApplication 
{
public:
	ScreenSaverApp();
	virtual void DispatchMessage(BMessage *msg,BHandler *target);
	void goBlank(void);
	void unBlank(void);
	void resetTimer(void);
	void setNewDuration(int newTimer);
	void setNewSaver(BMessage *newSSMessage);
private:
	int blankTime;
	int currentTime;
	thread_id threadID;

};

#endif //SCREEN_SAVER_H
