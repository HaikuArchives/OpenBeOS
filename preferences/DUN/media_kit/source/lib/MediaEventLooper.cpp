/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: MediaEventLooper.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaEventLooper.h>
#include "debug.h"

/*************************************************************
 * protected BMediaEventLooper
 *************************************************************/

/* virtual */
BMediaEventLooper::~BMediaEventLooper()
{
	UNIMPLEMENTED();
}

/* explicit */
BMediaEventLooper::BMediaEventLooper(uint32 apiVersion)
	: BMediaNode("XXX fixme")
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::NodeRegistered()
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::Start(bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::Stop(bigtime_t performance_time,
						bool immediate)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::Seek(bigtime_t media_time,
						bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::TimeWarp(bigtime_t at_real_time,
							bigtime_t to_performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ status_t
BMediaEventLooper::AddTimer(bigtime_t at_performance_time,
							int32 cookie)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ void
BMediaEventLooper::SetRunMode(run_mode mode)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaEventLooper::CleanUpEvent(const media_timed_event *event)
{
	UNIMPLEMENTED();
}


/* virtual */ bigtime_t
BMediaEventLooper::OfflineTime()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


/* virtual */ void
BMediaEventLooper::ControlLoop()
{
	UNIMPLEMENTED();
}


thread_id
BMediaEventLooper::ControlThread()
{
	UNIMPLEMENTED();
	thread_id dummy;

	return dummy;
}

/*************************************************************
 * protected BMediaEventLooper
 *************************************************************/


BTimedEventQueue *
BMediaEventLooper::EventQueue()
{
	UNIMPLEMENTED();
	return NULL;
}


BTimedEventQueue *
BMediaEventLooper::RealTimeQueue()
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BMediaEventLooper::Priority() const
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


int32
BMediaEventLooper::RunState() const
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


bigtime_t
BMediaEventLooper::EventLatency() const
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bigtime_t
BMediaEventLooper::BufferDuration() const
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bigtime_t
BMediaEventLooper::SchedulingLatency() const
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


status_t
BMediaEventLooper::SetPriority(int32 priority)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BMediaEventLooper::SetRunState(run_state state)
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::SetEventLatency(bigtime_t latency)
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::SetBufferDuration(bigtime_t duration)
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::SetOfflineTime(bigtime_t offTime)
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::Run()
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::Quit()
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::DispatchEvent(const media_timed_event *event,
								 bigtime_t lateness,
								 bool realTimeEvent)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * private BMediaEventLooper
 *************************************************************/


int32
BMediaEventLooper::_ControlThreadStart(void *arg)
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


void
BMediaEventLooper::_CleanUpEntry(const media_timed_event *event,
								 void *context)
{
	UNIMPLEMENTED();
}


void
BMediaEventLooper::_DispatchCleanUp(const media_timed_event *event)
{
	UNIMPLEMENTED();
}

/*
// unimplemented
BMediaEventLooper::BMediaEventLooper(const BMediaEventLooper &)
BMediaEventLooper &BMediaEventLooper::operator=(const BMediaEventLooper &)
*/

/*************************************************************
 * protected BMediaEventLooper
 *************************************************************/


status_t
BMediaEventLooper::DeleteHook(BMediaNode *node)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BMediaEventLooper
 *************************************************************/

status_t BMediaEventLooper::_Reserved_BMediaEventLooper_0(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_1(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_2(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_3(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_4(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_5(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_6(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_7(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_8(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_9(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_10(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_11(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_12(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_13(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_14(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_15(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_16(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_17(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_18(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_19(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_20(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_21(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_22(int32 arg,...) { return 0; }
status_t BMediaEventLooper::_Reserved_BMediaEventLooper_23(int32 arg,...) { return 0; }

