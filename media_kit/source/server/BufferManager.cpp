#include <MediaDefs.h>
#include <Autolock.h>
#include "BufferManager.h"
#include "../lib/headers/SharedBufferList.h"
#include "../lib/headers/debug.h"

BufferManager::BufferManager()
{
	fSharedBufferList = _shared_buffer_list::Clone();
	fAreaId = area_for(fSharedBufferList);
	fBufferList = NULL;
	fLocker = new BLocker("buffer manager locker");
	fNextBufferId = 1;
}

BufferManager::~BufferManager()
{
	fSharedBufferList->Unmap();
	delete fLocker;
}

area_id
BufferManager::SharedBufferListID()
{
	return fAreaId;
}

status_t	
BufferManager::RegisterBuffer(team_id teamid, media_buffer_id bufferid,
							  size_t *size, int32 *flags, size_t *offset, area_id *area)
{
	BAutolock lock(fLocker);
	
	_buffer_list *list;
	_team_list *team;
	
	for (list = fBufferList; list; list = list->next)
		if (list->id == bufferid) {
			team 		= new _team_list;
			team->team 	= teamid;
			team->next 	= list->teams;
			list->teams = team;
			*area 		= list->area;
			*offset 	= list->offset;
			*size 		= list->size, 
			*flags 		= list->flags;
			return B_OK;
		}

	TRACE("failed to register buffer! team = 0x%08x, bufferid = 0x%08x\n",(int)teamid,(int)bufferid);
	return B_ERROR;
}

status_t
BufferManager::RegisterBuffer(team_id teamid, size_t size, int32 flags, size_t offset, area_id area,
							  media_buffer_id *bufferid)
{
	BAutolock lock(fLocker);
	
	void *adr;
	area_id newarea;

	newarea = clone_area("media_server buffer",&adr,B_ANY_ADDRESS,B_READ_AREA | B_WRITE_AREA,area);
	if (newarea <= B_OK) {
		TRACE("failed to clone buffer! team = 0x%08x, areaid = 0x%08x, offset = 0x%08x, size = 0x%08x\n",(int)teamid,(int)area,(int)offset,(int)size);
		return B_ERROR;
	}

	*bufferid = fNextBufferId;

	_buffer_list *list;
	list = new _buffer_list;
	list->next 			= fBufferList;
	list->id 			= fNextBufferId;
	list->area 			= newarea;
	list->offset 		= offset;
	list->size 			= size;
	list->flags 		= flags;
	list->teams 		= new _team_list;
	list->teams->next 	= NULL;
	list->teams->team 	= teamid;
	fBufferList = list;

	fNextBufferId++;
	
	return B_OK;
}

status_t
BufferManager::UnregisterBuffer(team_id teamid, media_buffer_id bufferid)
{
	BAutolock lock(fLocker);

	_buffer_list *currentlist;
	_buffer_list **nextlist;
	_team_list *currentteam;
	_team_list **nextteam;
	
	for (nextlist = &fBufferList; (*nextlist); nextlist = &((*nextlist)->next)) {
		if ((*nextlist)->id == bufferid) {
			currentlist = *nextlist;
			*nextlist = (*nextlist)->next;
			for (nextteam = &(currentlist->teams); (*nextteam); nextteam = &((*nextteam)->next)) {
				if ((*nextteam)->team == teamid) {
					currentteam = *nextteam;
					*nextteam = (*nextteam)->next;
					delete currentteam;
					TRACE("team = 0x%08x removed from bufferid = 0x%08x\n",(int)teamid,(int)bufferid);
					break;
				}
			}
			if (currentlist->teams == NULL) {
				delete_area(currentlist->area);
				delete currentlist;
				TRACE("bufferid = 0x%08x removed\n",(int)bufferid);
			}
			return B_OK;
		}
	}
	TRACE("failed to unregister buffer! team = 0x%08x, bufferid = 0x%08x\n",(int)teamid,(int)bufferid);
	return B_ERROR;
}

