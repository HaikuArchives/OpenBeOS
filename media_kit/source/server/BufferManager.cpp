#include <MediaDefs.h>
#include "BufferManager.h"
#include "../lib/headers/SharedBufferList.h"

BufferManager::BufferManager()
{
	fBufferList = _shared_buffer_list::Clone();
	fAreaId = area_for(fBufferList);
}

BufferManager::~BufferManager()
{
	fBufferList->Unmap();
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
	return B_OK;
}

status_t
BufferManager::RegisterBuffer(team_id teamid, size_t size, int32 flags, size_t offset, area_id area,
							  media_buffer_id *bufferid)
{
	return B_OK;
}

status_t
BufferManager::UnregisterBuffer(team_id teamid, media_buffer_id bufferid)
{
	return B_OK;
}

