#include "InputServerFilter.h"

class SSISFilter: public BInputServerFilter
{
public:
	SSISFilter();
	virtual ~SSISFilter();
	virtual filter_result Filter(BMessage *msg, BList *outList);
private:
	uint32 first;
	bool enabled;
	bool toSend;
	BMessenger *ssApp;
};

