/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: TimedEventQueue.cpp
 *  DESCR: 
 ***********************************************************************/
#include <TimedEventQueue.h>
#include "debug.h"

/*************************************************************
 * struct media_timed_event
 *************************************************************/

media_timed_event::media_timed_event()
{
	UNIMPLEMENTED();
}


media_timed_event::media_timed_event(bigtime_t inTime,
									 int32 inType)
{
	UNIMPLEMENTED();
}


media_timed_event::media_timed_event(bigtime_t inTime,
									 int32 inType,
									 void *inPointer,
									 uint32 inCleanup)
{
	UNIMPLEMENTED();
}


media_timed_event::media_timed_event(bigtime_t inTime,
									 int32 inType,
									 void *inPointer,
									 uint32 inCleanup,
									 int32 inData,
									 int64 inBigdata,
									 char *inUserData,
									 size_t dataSize)
{
	UNIMPLEMENTED();
}


media_timed_event::media_timed_event(const media_timed_event &clone)
{
	UNIMPLEMENTED();
}


void
media_timed_event::operator=(const media_timed_event &clone)
{
	UNIMPLEMENTED();
}


media_timed_event::~media_timed_event()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * global operators
 *************************************************************/

bool operator==(const media_timed_event & a, const media_timed_event & b)
{
	UNIMPLEMENTED();
	return true;
}

bool operator!=(const media_timed_event & a, const media_timed_event & b)
{
	UNIMPLEMENTED();
	return true;
}

bool operator<(const media_timed_event & a, const media_timed_event & b)
{
	UNIMPLEMENTED();
	return true;
}

bool operator>(const media_timed_event & a, const media_timed_event &b)
{
	UNIMPLEMENTED();
	return true;
}


/*************************************************************
 * public BTimedEventQueue
 *************************************************************/


void *
BTimedEventQueue::operator new(size_t s)
{
	UNIMPLEMENTED();
	return NULL;
}


void
BTimedEventQueue::operator delete(void *p,
								  size_t s)
{
	UNIMPLEMENTED();
}


BTimedEventQueue::BTimedEventQueue()
{
	UNIMPLEMENTED();
}


BTimedEventQueue::~BTimedEventQueue()
{
	UNIMPLEMENTED();
}


status_t
BTimedEventQueue::AddEvent(const media_timed_event &event)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BTimedEventQueue::RemoveEvent(const media_timed_event *event)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BTimedEventQueue::RemoveFirstEvent(media_timed_event *outEvent)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bool
BTimedEventQueue::HasEvents() const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


int32
BTimedEventQueue::EventCount() const
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


const media_timed_event *
BTimedEventQueue::FirstEvent() const
{
	UNIMPLEMENTED();
	return NULL;
}


bigtime_t
BTimedEventQueue::FirstEventTime() const
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


const media_timed_event *
BTimedEventQueue::LastEvent() const
{
	UNIMPLEMENTED();
	return NULL;
}


bigtime_t
BTimedEventQueue::LastEventTime() const
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


const media_timed_event *
BTimedEventQueue::FindFirstMatch(bigtime_t eventTime,
								 time_direction direction,
								 bool inclusive,
								 int32 eventType)
{
	UNIMPLEMENTED();
	return NULL;
}


status_t
BTimedEventQueue::DoForEach(for_each_hook hook,
							void *context,
							bigtime_t eventTime,
							time_direction direction,
							bool inclusive,
							int32 eventType)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BTimedEventQueue::SetCleanupHook(cleanup_hook hook,
								 void *context)
{
	UNIMPLEMENTED();
}


status_t
BTimedEventQueue::FlushEvents(bigtime_t eventTime,
							  time_direction direction,
							  bool inclusive,
							  int32 eventType)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BTimedEventQueue
 *************************************************************/

/*
// unimplemented
BTimedEventQueue::BTimedEventQueue(const BTimedEventQueue &other)
BTimedEventQueue &BTimedEventQueue::operator=(const BTimedEventQueue &other)
*/

status_t BTimedEventQueue::_Reserved_BTimedEventQueue_0(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_1(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_2(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_3(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_4(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_5(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_6(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_7(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_8(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_9(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_10(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_11(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_12(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_13(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_14(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_15(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_16(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_17(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_18(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_19(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_20(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_21(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_22(void *, ...) { return 0; }
status_t BTimedEventQueue::_Reserved_BTimedEventQueue_23(void *, ...) { return 0; }
