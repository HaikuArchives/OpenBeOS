#include <Application.h>
#include <MediaRoster.h>
#include "misc.h"

BMediaRoster *roster;

int main()
{
	out("Basic BBufferProducer, BBufferConsumer, BMediaRoster test\n");
	out("for OpenBeOS by Marcus Overhagen <Marcus@Overhagen.de>\n\n");
	out("Creating BApplication now\n");
	BApplication app("application/x-vnd.OpenBeOS-NodeTest");	

	out("Creating MediaRoster...");
	roster = BMediaRoster::Roster();
	val(roster);
	
	wait();
	return 0;
}

