/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferIdCache.cpp
 *  DESCR: used to cache BBuffers to be received by 
 *         BBufferConsumer::BufferReceived()
 ***********************************************************************/

#include <Buffer.h>
#include "BufferIdCache.h"
#include "debug.h"

_buffer_id_cache::_buffer_id_cache() : 
	used(0),
	miss(0),
	hit(0),
	last(0)
{
	for (int i = 0; i < MAX_CACHED_BUFFER; i++) {
		info[i].buffer = 0;
		info[i].id = 0;
		info[i].lastused = 0;
	}
}

_buffer_id_cache::~_buffer_id_cache()
{
	for (int i = 0; i < MAX_CACHED_BUFFER; i++)
		if (info[i].buffer)
			delete info[i].buffer;
	TRACE("### _buffer_id_cache finished, %ld hit, %ld missed\n",hit,miss);
}
	
BBuffer *
_buffer_id_cache::GetBuffer(media_buffer_id id)
{
	if (id == 0)
		debugger("_buffer_id_cache::GetBuffer called with 0 id\n");
	
	last++;

	// try to find in cache		
	for (int i = 0; i < MAX_CACHED_BUFFER; i++) {
		if (info[i].id == id) {
			hit++;
			info[i].lastused = last;
			return info[i].buffer;
		}
	}
	
	miss++;

	// remove last recently used 	
	if (used == MAX_CACHED_BUFFER) {
		int32 maxused = last;
		int index = 0;
		for (int i = 0; i < MAX_CACHED_BUFFER; i++) {
			if (info[i].lastused < maxused) {
				maxused = info[i].lastused;
				index = i;
			}
		}
		info[index].buffer = NULL;
	}
	
	BBuffer *buffer;
	buffer_clone_info ci;
	ci.buffer = id;
	buffer = new BBuffer(0,ci);

	// insert into cache
	for (int i = 0; i < MAX_CACHED_BUFFER; i++) {
		if (info[i].buffer == NULL) {
			info[i].buffer = buffer;
			info[i].id = id;
			info[i].lastused = last;
			break;
		}
	}
	return buffer;	
}
