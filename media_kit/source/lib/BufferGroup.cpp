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

struct _shared_buffer_list
{
};

/*************************************************************
 * _buffer_id_cache
 *************************************************************/

class _buffer_id_cache
{
};

/*************************************************************
 * public BBufferGroup
 *************************************************************/

BBufferGroup::BBufferGroup(size_t size,
						   int32 count,
						   uint32 placement,
						   uint32 lock)
{
	void *start_addr
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

	// XXX we ignore the placement parameter
	if (placement != B_ANY_ADDRESS)
		TRACE("placement != B_ANY_ADDRESS (0x%08lx)\n",placement);
	
	// first we roundup for a better placement in memory
	size = (size + 63) & ~63;

	// now we create the area
	area_size = ((size * count) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);

	buffer_area = create_area("some buffers area", &start_addr,B_ANY_ADDRESS,area_size,lock,B_READ_AREA | B_WRITE_AREA);
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

	// this one creates buffers from buffer_ids passed 
	// by the application.

	fBufferCount = count;

	buffer_clone_info bci;
	for (int32 i = 0; i < count; i++) {	
		bci.buffer = buffers[i];
		buffer = new BBuffer(bci);
		if (0 == buffer->Data()) {
			// BBuffer::Data() will return 0 if an error occured
			TRACE("error while creating buffer\n");
			fInitError = B_ERROR;
			break;
		}
		fBufferList->AddBuffer(buffer);
	}
}


BBufferGroup::~BBufferGroup()
{
	UNIMPLEMENTED();
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
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
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
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferGroup::ReclaimAllBuffers()
{
	UNIMPLEMENTED();
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


