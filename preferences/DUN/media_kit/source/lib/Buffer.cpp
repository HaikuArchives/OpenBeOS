/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: Buffer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaDefs.h>
#include <Buffer.h>
#include "debug.h"

struct _shared_buffer_list
{
};

/*************************************************************
 * public struct buffer_clone_info
 *************************************************************/

buffer_clone_info::buffer_clone_info()
{
	UNIMPLEMENTED();
}


buffer_clone_info::~buffer_clone_info()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BBuffer
 *************************************************************/

void *
BBuffer::Data()
{
	UNIMPLEMENTED();
	return NULL;
}


size_t
BBuffer::SizeAvailable()
{
	UNIMPLEMENTED();
	size_t dummy;

	return dummy;
}


size_t
BBuffer::SizeUsed()
{
	UNIMPLEMENTED();
	size_t dummy;

	return dummy;
}


void
BBuffer::SetSizeUsed(size_t size_used)
{
	UNIMPLEMENTED();
}


uint32
BBuffer::Flags()
{
	UNIMPLEMENTED();
	uint32 dummy;

	return dummy;
}


void
BBuffer::Recycle()
{
	UNIMPLEMENTED();
}


buffer_clone_info
BBuffer::CloneInfo() const
{
	UNIMPLEMENTED();
	buffer_clone_info dummy;

	return dummy;
}


media_buffer_id
BBuffer::ID()
{
	UNIMPLEMENTED();
	media_buffer_id dummy;

	return dummy;
}


media_type
BBuffer::Type()
{
	UNIMPLEMENTED();
	media_type dummy;

	return dummy;
}


media_header *
BBuffer::Header()
{
	UNIMPLEMENTED();
	return NULL;
}


media_audio_header *
BBuffer::AudioHeader()
{
	UNIMPLEMENTED();
	return NULL;
}


media_video_header *
BBuffer::VideoHeader()
{
	UNIMPLEMENTED();
	return NULL;
}


size_t
BBuffer::Size()
{
	UNIMPLEMENTED();
	// deprecated; use SizeAvailable()
	return SizeAvailable();
}

/*************************************************************
 * private BBuffer
 *************************************************************/

BBuffer::BBuffer(area_id area,
				 size_t offset,
				 size_t size,
				 int32 flags)
{
	UNIMPLEMENTED();
}


BBuffer::BBuffer(media_header *_mHeader)
{
	UNIMPLEMENTED();
}


BBuffer::~BBuffer()
{
	UNIMPLEMENTED();
}


BBuffer::BBuffer()
{
	UNIMPLEMENTED();
}


BBuffer::BBuffer(const BBuffer &clone)
{
	UNIMPLEMENTED();
}


BBuffer &
BBuffer::operator=(const BBuffer &clone)
{
	UNIMPLEMENTED();
	return *this;
}


void
BBuffer::SetOwnerArea(area_id owner)
{
	UNIMPLEMENTED();
}


void
BBuffer::SetHeader(media_header *header)
{
	UNIMPLEMENTED();
}

/* explicit */
BBuffer::BBuffer(const buffer_clone_info &info)
{
	UNIMPLEMENTED();
}


void
BBuffer::SetGroupOwnerPort(port_id port)
{
	UNIMPLEMENTED();
}


void
BBuffer::SetCurrentOwner(port_id port)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BSmallBuffer
 *************************************************************/

BSmallBuffer::BSmallBuffer()
{
	UNIMPLEMENTED();
}


size_t
BSmallBuffer::SmallBufferSizeLimit()
{
	UNIMPLEMENTED();
	size_t dummy;

	return dummy;
}


