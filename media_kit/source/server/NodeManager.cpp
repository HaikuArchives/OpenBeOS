#include <OS.h>
#include <Message.h>
#include <Messenger.h>
#include <MediaDefs.h>
#include "NodeManager.h"
#include "TList.h"

NodeManager::NodeManager() :
	nextaddonid(1)
{
}

NodeManager::~NodeManager()
{
}

void 
NodeManager::RegisterAddon(media_addon_id *newid)
{
	*newid = nextaddonid++;
}

void
NodeManager::UnregisterAddon(media_addon_id id)
{
	RemoveDormantFlavorInfo(id);
	// unload the image once it's no longer used (refcounting!)
}

void
NodeManager::AddDormantFlavorInfo(const dormant_flavor_info &dfi)
{
}

void
NodeManager::RemoveDormantFlavorInfo(media_addon_id id)
{
}
