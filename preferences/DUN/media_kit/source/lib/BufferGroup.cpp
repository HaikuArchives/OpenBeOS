/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferGroup.cpp
 *  DESCR: 
 ***********************************************************************/
#include <BufferGroup.h>
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
	UNIMPLEMENTED();
}

/* explicit */
BBufferGroup::BBufferGroup()
{
	UNIMPLEMENTED();
}


BBufferGroup::BBufferGroup(int32 count,
						   const media_buffer_id *buffers)
{
	UNIMPLEMENTED();
}


BBufferGroup::~BBufferGroup()
{
	UNIMPLEMENTED();
}


status_t
BBufferGroup::InitCheck()
{
	UNIMPLEMENTED();

	return B_OK;
}


status_t
BBufferGroup::AddBuffer(const buffer_clone_info &info,
						BBuffer **out_buffer)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
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
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
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


