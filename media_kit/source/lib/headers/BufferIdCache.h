#ifndef _BUFFER_ID_CACHE_H_
#define _BUFFER_ID_CACHE_H_

/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferIdCache.h
 *  DESCR: used to cache BBuffers to be received by 
 *         BBufferConsumer::BufferReceived()
 ***********************************************************************/

class _buffer_id_cache
{
public:
	_buffer_id_cache();
	~_buffer_id_cache();
	
	BBuffer *GetBuffer(media_buffer_id id);

private:
	enum { MAX_CACHED_BUFFER = 17 };
	struct _buffer_id_info
	{
		BBuffer *buffer;
		media_buffer_id id;
		bigtime_t lastused;
	};
	_buffer_id_info info[MAX_CACHED_BUFFER];
	int32 used;
	int32 miss;
	int32 hit;
};

#endif