/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SharedBufferList.cpp
 *  DESCR: 
 ***********************************************************************/
#include <Buffer.h>
#include "SharedBufferList.h"
#include "debug.h"


status_t
_shared_buffer_list::Init()
{
	CALLED();
	locker_atom = 0;
	locker_sem = create_sem(0,"shared buffer list lock");
	for (int i = 0; i < MAX_BUFFER; i++) {
		info[i].id = -1;
		info[i].buffer = 0;
		info[i].reclaim_sem = 0;
		info[i].reclaimed = false;
	}
	
	return B_OK;
}

_shared_buffer_list *
_shared_buffer_list::Clone(area_id id)
{
	CALLED();
	// if id == -1, we are in the media_server team, 
	// and create the initial list, else we clone it

	_shared_buffer_list *adr;
	status_t status;

	if (id == -1) {
		size_t size = ((sizeof(_shared_buffer_list)) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);
		status = create_area("shared buffer list",(void **)&adr,B_ANY_KERNEL_ADDRESS,size,B_LAZY_LOCK,B_READ_AREA | B_WRITE_AREA);
		if (status >= B_OK) {
			status = adr->Init();
			if (status != B_OK)
				delete_area(area_for(adr));
		}
	} else {
		status = clone_area("shared buffer list clone",(void **)&adr,B_ANY_KERNEL_ADDRESS,B_READ_AREA | B_WRITE_AREA,id);
	}
	
	return (status < B_OK) ? NULL : adr;
}

void
_shared_buffer_list::Unmap()
{
	// unmap the memory used by this struct
	// XXX is this save?
	area_id id;
	id = area_for(this);
	if (id >= B_OK)
		delete_area(id);
}

void
_shared_buffer_list::Terminate(sem_id group_reclaim_sem)
{
	CALLED();

	// delete all BBuffers of this group, then unmap from memory
	
	Lock();

	for (int32 i = 0; i < buffercount; i++) {
		if (info[i].reclaim_sem == group_reclaim_sem) {
			// delete the associated buffer
			delete info[i].buffer;
			// fill the gap in the list with the last element
			// and adjust i and buffercount 
			info[i--] = info[--buffercount];
		}
	}
	
	Unlock();
	
	Unmap();
}

void 
_shared_buffer_list::Lock()
{ 
	if (atomic_add(&locker_atom, 1) > 0)
		while (B_INTERRUPTED == acquire_sem(locker_sem))
			;
}

void
_shared_buffer_list::Unlock()
{ 
	if (atomic_add(&locker_atom, -1) > 1)
		release_sem(locker_sem);
}

void
_shared_buffer_list::AddBuffer(sem_id group_reclaim_sem, BBuffer *buffer)
{
	CALLED();
	Lock();
	
	if (buffercount == MAX_BUFFER) {
		Unlock();
		debugger("we are doomed");
	}

	info[buffercount].id = buffer->ID();
	info[buffercount].buffer = buffer;
	info[buffercount].reclaim_sem = group_reclaim_sem;
	info[buffercount].reclaimed = true;
	buffercount++;

	Unlock();
}

status_t	
_shared_buffer_list::RequestBuffer(sem_id group_reclaim_sem, int32 buffers_in_group, size_t size, media_buffer_id wantID, BBuffer **buffer, bigtime_t timeout)
{
	CALLED();
	// we always search for a buffer from the group indicated by group_reclaim_sem first
	// if "size" != 0, we search for a buffer that is "size" bytes or larger
	// if "wantID" != 0, we search for a buffer with this id
	// if "*buffer" != NULL, we search for a buffer at this address
	// if we found a buffer, we also need to mark it in all other groups as requested
	// and also once need to release the reclaim_sem of the other groups
	
	status_t status;
	int32 count;

	if (timeout != B_INFINITE_TIMEOUT)	
		timeout += system_time();
		
	// with each itaration we request one more buffer, since we need to skip the buffers that don't fit the request
	count = 1;
	
	do {
		while (B_INTERRUPTED == (status = acquire_sem_etc(group_reclaim_sem, count, (timeout != B_INFINITE_TIMEOUT) ? B_ABSOLUTE_TIMEOUT : 0, timeout)))
			;
		if (status != B_OK)
			return status;
			
		Lock();
		
		for (int32 i = 0; i < buffercount; i++) {
			// we need a BBuffer from the group, and it must be reclaimed
			if (info[i].reclaim_sem == group_reclaim_sem && info[i].reclaimed) {
				if (
					  (size != 0 && size <= info[i].buffer->SizeAvailable()) ||
					  (*buffer != 0 && info[i].buffer == *buffer) ||
					  (wantID != 0 && info[i].id == wantID)
				   ) {
				   	// we found a buffer
					info[i].reclaimed = false;
					*buffer = info[i].buffer;
					// if we requested more than one buffer, release the rest
					if (count > 1)
						release_sem_etc(group_reclaim_sem, count - 1, B_DO_NOT_RESCHEDULE);
					
					RequestBufferInOtherGroups(group_reclaim_sem, info[i].buffer->ID());

					Unlock();
					return B_OK;
				}
			}
		}

		release_sem_etc(group_reclaim_sem, count, B_DO_NOT_RESCHEDULE);
		Unlock();

		// prepare to request one more buffer next time
		count++;
	} while (count <= buffers_in_group);

	return B_ERROR;
}

void 
_shared_buffer_list::RequestBufferInOtherGroups(sem_id group_reclaim_sem, media_buffer_id id)
{
	for (int32 i = 0; i < buffercount; i++) {
		// find buffers belonging to other groups
		if (info[i].id == id && info[i].reclaim_sem != group_reclaim_sem) {

			// and mark them as requested 
			// XXX this can deadlock if BBuffers with same media_buffer_id
			// XXX exist in more than one BBufferGroup, and RequestBuffer()
			// XXX is called on both groups (which should not be done).
			while (B_INTERRUPTED == acquire_sem(info[i].reclaim_sem))
				;

			if (info[i].reclaimed == false) {
				TRACE("Error, BBuffer 0x%08x, id = 0x%08x not reclaimed while requesting\n",(int)info[i].buffer,(int)id);
				continue;
			}
			
			info[i].reclaimed = false;
		}
	}
}

void
_shared_buffer_list::ReclaimBuffer(BBuffer *buffer)
{
	CALLED();
	
	int debug_reclaim = 0;
	
	media_buffer_id id = buffer->ID();

	Lock();
	for (int32 i = 0; i < buffercount; i++) {
		// find the buffer id, and reclaim it in all groups it belongs to
		if (info[i].id == id) {
			debug_reclaim++;
			if (info[i].reclaimed) {
				TRACE("Error, BBuffer 0x%08x, id = 0x%08x already reclaimed\n",(int)buffer,(int)id);
				break;
			}
			info[i].reclaimed = true;
			release_sem_etc(info[i].reclaim_sem, 1, B_DO_NOT_RESCHEDULE);
		}
	}
	Unlock();
	
	if (debug_reclaim == 0)
		TRACE("Error, BBuffer 0x%08x, id = 0x%08x NOT reclaimed\n",(int)buffer,(int)buffer->ID());
}

status_t	
_shared_buffer_list::GetBufferList(sem_id group_reclaim_sem, int32 buf_count, BBuffer **out_buffers)
{
	CALLED();

	int32 found;
	
	found = 0;
	Lock();

	for (int32 i = 0; i < buffercount; i++)
		if (info[i].reclaim_sem == group_reclaim_sem) {
			out_buffers[found++] = info[i].buffer;
			if (found == buf_count)
				break;
		}
	
	Unlock();

	return (found == buf_count) ? B_OK : B_ERROR;
}

