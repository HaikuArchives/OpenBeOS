#ifndef _SHARED_BUFFER_LIST_H_
#define _SHARED_BUFFER_LIST_H_

/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SharedBufferList.cpp
 *  DESCR: used for BBufferGroup and BBuffer management
 ***********************************************************************/

#define MAX_BUFFER 512 			// this is probably very evil

struct _shared_buffer_info
{
	media_buffer_id id;
	BBuffer *		buffer;
	bool 			reclaimed;
	// the reclaim_sem belonging to the BBufferGroup of this BBuffer
	// also used as a unique identifier of the group
	sem_id			reclaim_sem; 
};


// created in the media server, cloned into 
// each BBufferGroup (visible in all address spaces / teams)
struct _shared_buffer_list
{
	sem_id		locker_sem;
	int32		locker_atom;
	
	// always only the first "buffercount" entries in the "info" array are used
	int32		buffercount;
	_shared_buffer_info info[MAX_BUFFER];

	void 		AddBuffer(sem_id group_reclaim_sem, BBuffer *buffer);
	status_t	RequestBuffer(sem_id group_reclaim_sem, int32 buffers_in_group, size_t size, media_buffer_id wantID, BBuffer **buffer, bigtime_t timeout);
	status_t	GetBufferList(sem_id group_reclaim_sem, int32 buf_count, BBuffer **out_buffers);
	
		
	status_t 	Init();

	static _shared_buffer_list *Clone(area_id id = -1);
	void 		Terminate(sem_id group_reclaim_sem);

	void 		Lock();
	void 		Unlock();
};

#endif
