/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferGroup.cpp
 *  DESCR: 
 ***********************************************************************/
#include <BufferGroup.h>
#include <Buffer.h>
#include "debug.h"

/*************************************************************
 * _shared_buffer_list
 *************************************************************/

#define MAX_BUFFER 256 // this is probably evil

struct _shared_buffer_info
{
	media_buffer_id id;
	team_id 		team;
	BBuffer *		buffer;
	BBufferGroup *	group;
	bool 			reclaimed;
};

// created in the media server, cloned into 
// each BBufferGroup (visible in all address spaces / teams)
struct _shared_buffer_list
{
	sem_id		locker_sem;
	int32		locker_atom;

	sem_id		recycled_sem;
	
	_shared_buffer_info info[MAX_BUFFER];
	
	void 		AddBuffer(BBuffer *buffer);
	
		
	status_t 	Init();

	static _shared_buffer_list *Clone(area_id id = -1);
	void 		Terminate();

	void 		Lock();
	void 		Unlock();
	
};

status_t
_shared_buffer_list::Init()
{
	locker_atom = 0;
	locker_sem = create_sem(0,"shared buffer list lock");
	recycled_sem = create_sem(0,"shared buffer free count");
	for (int i = 0; i < MAX_BUFFER; i++) {
		info.id = -1;
		info.team = 0;
		info.buffer = 0;
		info.group = 0;
		info.reclaimed = false;
	}
	
	return B_OK;
}

_shared_buffer_list *
_shared_buffer_list::Clone(area_id id)
{
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
_shared_buffer_list::Terminate()
{
	area_id id;
	id = area_for(this);
	if (id >= B_OK)
		delete_area(id);
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
_shared_buffer_list::AddBuffer(BBuffer *buffer)
{
	Lock();
	Unlock();
}



/*************************************************************
 * _buffer_id_cache
 *************************************************************/

class _buffer_id_cache
{
};

/*************************************************************
 * private BBufferGroup
 *************************************************************/

status_t				
BBufferGroup::InitBufferGroup()
{
	area_id id;

	//ask media_server to get the area_id of the shared buffer list
	id = 0;

	fRequestError = B_OK;
	fBufferCount = 0;
	fBufferList = _shared_buffer_list::Clone(id);
	fInitError = (fBufferList != NULL) ? B_OK : B_ERROR;

	return fInitError;
}

/*************************************************************
 * public BBufferGroup
 *************************************************************/

BBufferGroup::BBufferGroup(size_t size,
						   int32 count,
						   uint32 placement,
						   uint32 lock)
{
	void *start_addr;
	area_id buffer_area;
	size_t area_size;
	buffer_clone_info bci;
	BBuffer *buffer;

	CALLED();
	if (InitBufferGroup() != B_OK)
		return;
		
	// This one is easy. We need to create "count" BBuffers,
	// each one "size" bytes large. They all go into one 
	// area, with "placement" and "lock" attributes.
	// The BBuffers created will clone the area, and
	// then we delete our area. This way BBuffers are
	// independent from the BBufferGroup

	// don't allow all placement parameter values
	if (placement != B_ANY_ADDRESS && placement != B_ANY_KERNEL_ADDRESS) {
		TRACE("placement != B_ANY_ADDRESS && placement != B_ANY_KERNEL_ADDRESS (0x%08lx)\n",placement);
		placement = B_ANY_ADDRESS;
	}
	
	// first we roundup for a better placement in memory
	size = (size + 63) & ~63;

	// now we create the area
	area_size = ((size * count) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);

	buffer_area = create_area("some buffers area", &start_addr,placement,area_size,lock,B_READ_AREA | B_WRITE_AREA);
	if (buffer_area < B_OK) {
		TRACE("failed to allocate %ld bytes area\n",area_size);
		fInitError = (status_t)buffer_area;
		return;
	}
	
	fBufferCount = count;

	for (int32 i = 0; i < count; i++) {	
		bci.area = buffer_area;
		bci.offset = i * size;
		bci.size = size;
		buffer = new BBuffer(bci);
		if (0 == buffer->Data()) {
			// BBuffer::Data() will return 0 if an error occured
			TRACE("error while creating buffer\n");
			delete buffer;
			fInitError = B_ERROR;
			break;
		}
		fBufferList->AddBuffer(buffer);
	}

	delete_area(buffer_area);
}

/* explicit */
BBufferGroup::BBufferGroup()
{
	CALLED();
	if (InitBufferGroup() != B_OK)
		return;
		
	// this one simply creates an empty BBufferGroup
}


BBufferGroup::BBufferGroup(int32 count,
						   const media_buffer_id *buffers)
{
	CALLED();
	if (InitBufferGroup() != B_OK)
		return;

	// this one creates "BBuffer"s from "media_buffer_id"s passed 
	// by the application.

	fBufferCount = count;

	buffer_clone_info bci;
	BBuffer *buffer;
	for (int32 i = 0; i < count; i++) {	
		bci.buffer = buffers[i];
		buffer = new BBuffer(bci);
		if (0 == buffer->Data()) {
			// BBuffer::Data() will return 0 if an error occured
			TRACE("error while creating buffer\n");
			delete buffer;
			fInitError = B_ERROR;
			break;
		}
		fBufferList->AddBuffer(buffer);
	}
}


BBufferGroup::~BBufferGroup()
{
	CALLED();
	fBufferList->Terminate();
}


status_t
BBufferGroup::InitCheck()
{
	CALLED();
	return fInitError;
}


status_t
BBufferGroup::AddBuffer(const buffer_clone_info &info,
						BBuffer **out_buffer)
{
	CALLED();
	BBuffer *buffer;	
	buffer = new BBuffer(info);
	if (0 == buffer->Data()) {
		// BBuffer::Data() will return 0 if an error occured
		TRACE("error while creating buffer\n");
		delete buffer;
		return B_ERROR;
	}
	fBufferList->AddBuffer(buffer);
	if (out_buffer != 0)
		*out_buffer = buffer;

	return B_OK;
}


BBuffer *
BBufferGroup::RequestBuffer(size_t size,
							bigtime_t timeout)
{
	UNIMPLEMENTED();
	return NULL;
}


status_t
BBufferGroup::RequestBuffer(BBuffer *buffer,
							bigtime_t timeout)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::RequestError()
{
	CALLED();
	return fRequestError;
}


status_t
BBufferGroup::CountBuffers(int32 *out_count)
{
	CALLED();
	if (fInitError != B_OK)
		return fInitError;

	*out_count = fBufferCount;
	return B_OK;
}


status_t
BBufferGroup::GetBufferList(int32 buf_count,
							BBuffer **out_buffers)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::WaitForBuffers()
{
	CALLED();
	return B_OK;
}


status_t
BBufferGroup::ReclaimAllBuffers()
{
	CALLED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BBufferGroup
 *************************************************************/

/* static */ status_t
BBufferGroup::_entry_reclaim(void *)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/* not implemented */
/*
BBufferGroup::BBufferGroup(const BBufferGroup &)
BBufferGroup & BBufferGroup::operator=(const BBufferGroup &)
*/

status_t
BBufferGroup::IBufferGroup()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::AddToList(BBuffer *buffer)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::AddBuffersTo(BMessage *message,
						   const char *name,
						   bool needLock)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::_RequestBuffer(size_t size,
							 media_buffer_id wantID,
							 BBuffer **buffer,
							 bigtime_t timeout)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BBufferGroup::SetOwnerPort(port_id owner)
{
	UNIMPLEMENTED();
}


bool
BBufferGroup::CanReclaim()
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


void
BBufferGroup::WillReclaim()
{
	UNIMPLEMENTED();
}


status_t
BBufferGroup::Lock()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::Unlock()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


