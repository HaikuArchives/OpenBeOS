#include "SSInputFilter.h"
#include "Messenger.h"
#include "OS.h"

extern "C" _EXPORT BInputServerFilter* instantiate_input_filter();


SSISFilter::SSISFilter()
{
	first=real_time_clock();
	enabled=false;
	toSend=false;
	ssApp=NULL;
}

BInputServerFilter* instantiate_input_filter() 
{ 
	return (new SSISFilter()); 
}

filter_result SSISFilter::Filter(BMessage *msg,BList *outList)
{
	status_t err;
	toSend=true;
	if (real_time_clock()>first+4) // Only check every 5 seconds or so
		{
		if (!enabled)
			{
			delete ssApp;
			ssApp=new BMessenger("application/x-vnd.OBOS-ScreenSaverApp",-1,&err);
			if (err==B_OK)
				enabled=true;
			}
		
		if (enabled)
			{
			if (B_OK != ssApp->SendMessage((int32)('1'))) // Send a reset message. This protocol sucks.
				{
				enabled=false;
				}
			else
				{
				first=real_time_clock();
				toSend=false;
				}
			}
		}


}

SSISFilter::~SSISFilter()
{
	;
}
