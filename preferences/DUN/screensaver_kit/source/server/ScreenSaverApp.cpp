#ifndef SCREEN_SAVER_H
#include "ScreenSaverApp.h"
#endif
#include <stdio.h>
#include "Screen.h"
#include "image.h"

extern int32 screenSaverThread(void *);
class BScreenSaver;

SSstates nextAction; 
int32 frame;
BView *view;
BScreenSaver *saver;
sem_id ssSem;
SSAwindow *win;

int main(int, char**)
{	
	ScreenSaverApp myApplication;
	myApplication.SetPulseRate(1000000); // Fire once per second.
	ssSem=create_sem(0,"ScreenSaverSemaphore");
	nextAction=STOP;
	myApplication.Run();
	return(0);
}

ScreenSaverApp::ScreenSaverApp() : BApplication("application/x-vnd.OBOS-ScreenSaverApp")
{
	printf ("Valid messages are:\n");
	printf ("%d: GOBLANK\n",GOBLANK);
	printf ("%d: UNBLANK\n",UNBLANK);
	printf ("%d: RESET\n",RESET);
	printf ("%d: NEWDURATION\n",NEWDURATION);
	printf ("%d: SETNEWSAVER\n",SETNEWSAVER);
	blankTime=10; // This is temporary
	currentTime=0;
	win=NULL;
}

void ScreenSaverApp::DispatchMessage(BMessage *msg,BHandler *target)
{
	BScreenSaver *(*instantiate)(BMessage *, image_id );
	switch (msg->what)
		{
		case B_READY_TO_RUN:
			{
			threadID=spawn_thread(screenSaverThread,"Screen Saver Thread",0,NULL);
			image_id addon_image;
			addon_image = load_add_on("/boot/beos/system/add-ons/Screen Savers/Lissart");
			if (addon_image<0)
				{
				printf ("Unable to open the add-on\n");
				printf ("add on image = %x!\n",addon_image);
				}
			else
				if (B_OK != get_image_symbol(addon_image, "instantiate_screen_saver", B_SYMBOL_TYPE_TEXT,(void **) &instantiate))
					{
					printf ("Unable to find the instantiator\n");
					printf ("Error = %d\n",get_image_symbol(addon_image, "instantiate_screen_saver", B_SYMBOL_TYPE_TEXT,(void **) &instantiate));
					}
				else
					{
					saver=instantiate(msg,addon_image);
					BScreen theScreen(B_MAIN_SCREEN_ID);
					printf ("New window about to be made!\n");
					win=new SSAwindow(theScreen.Frame());
					printf ("New window = %x!\n",win);

					resume_thread(threadID);
					}
			}
			break;
		case GOBLANK:
			goBlank();
			break;
		case UNBLANK:
			unBlank();
			currentTime=0;
			break;
		case RESET:
			resetTimer();
			break;
		case NEWDURATION:
			int32 dur;
			msg->FindInt32("duration",&dur);
			setNewDuration(dur);
			break;
		case SETNEWSAVER:
			setNewSaver(msg);
			break;
		case B_PULSE:
			printf ("Current time = %d, blank at %d\n",currentTime,blankTime);
			if (++currentTime==blankTime)
				goBlank();
			break;
		case B_QUIT_REQUESTED:
			nextAction=EXIT;
			status_t dummy; // The thread has no return value
			wait_for_thread(threadID,&dummy);
			BApplication::QuitRequested();
			break;
		default:
			printf ("Unknown message! %d\n",msg->what);
			break;
		}
}

void ScreenSaverApp::goBlank(void)
{
	if (win)
		{
		printf ( "Blank!\n" ) ;
		win->SetFullScreen(true);
		win->Show(); 
		win->Sync(); 
		HideCursor();
		nextAction=DIRECTDRAW;
		release_sem(ssSem); // Tell the drawing thread to go and draw.
		}
}

void ScreenSaverApp::unBlank(void)
{ 
	if (win  && (nextAction == DIRECTDRAW))
		{
		win->SetFullScreen(false);
		win->Hide(); 
		win->Sync(); 
		ShowCursor();
		nextAction=STOP;
		}
	printf ( "Not - Blank!\n" ) ;
}

void ScreenSaverApp::resetTimer(void)
{
	currentTime=0;
	printf ( "timerReset!\n" ) ;
}

void ScreenSaverApp::setNewDuration(int newTimer)
{
	printf ( "new timer duration = %d\n" ,newTimer) ;
	blankTime=newTimer;
}

void ScreenSaverApp::setNewSaver(BMessage *newSSMessage)
{
	newSSMessage->PrintToStream();
}
