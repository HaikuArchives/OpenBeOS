#include "Globals.h"
#include "print_server.h"

#include <Roster.h>

status_t GetPrinterServerMessenger(BMessenger& msgr)
{
		// If print server is not yet running, start it
	if (!be_roster->IsRunning(B_PSRV_APP_SIGNATURE))
		be_roster->Launch(B_PSRV_APP_SIGNATURE);
	
	msgr = BMessenger(B_PSRV_APP_SIGNATURE);

	return msgr.IsValid() ? B_OK : B_ERROR;
}
