/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: TimeSource.cpp
 *  DESCR: 
 ***********************************************************************/
#include <TimeSource.h>
#include "debug.h"

/*************************************************************
 * protected BTimeSource
 *************************************************************/

BTimeSource::~BTimeSource()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BTimeSource
 *************************************************************/

status_t
BTimeSource::SnoozeUntil(bigtime_t performance_time,
						 bigtime_t with_latency,
						 bool retry_signals)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bigtime_t
BTimeSource::Now()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bigtime_t
BTimeSource::PerformanceTimeFor(bigtime_t real_time)
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bigtime_t
BTimeSource::RealTimeFor(bigtime_t performance_time,
						 bigtime_t with_latency)
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bool
BTimeSource::IsRunning()
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


status_t
BTimeSource::GetTime(bigtime_t *performance_time,
					 bigtime_t *real_time,
					 float *drift)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bigtime_t
BTimeSource::RealTime()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


status_t
BTimeSource::GetStartLatency(bigtime_t *out_latency)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * protected BTimeSource
 *************************************************************/


BTimeSource::BTimeSource()
	: BMediaNode("XXX fixme")
{
	UNIMPLEMENTED();

	AddNodeKind(B_TIME_SOURCE);
}


status_t
BTimeSource::HandleMessage(int32 message,
						   const void *data,
						   size_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BTimeSource::PublishTime(bigtime_t performance_time,
						 bigtime_t real_time,
						 float drift)
{
	UNIMPLEMENTED();
}


void
BTimeSource::BroadcastTimeWarp(bigtime_t at_real_time,
							   bigtime_t new_performance_time)
{
	UNIMPLEMENTED();
}


void
BTimeSource::SendRunMode(run_mode mode)
{
	UNIMPLEMENTED();
}


void
BTimeSource::SetRunMode(run_mode mode)
{
	UNIMPLEMENTED();
}
/*************************************************************
 * private BTimeSource
 *************************************************************/

/*
//unimplemented
BTimeSource::BTimeSource(const BTimeSource &clone)
BTimeSource &BTimeSource::operator=(const BTimeSource &clone)
*/

status_t BTimeSource::_Reserved_TimeSource_0(void *) { return 0; }
status_t BTimeSource::_Reserved_TimeSource_1(void *) { return 0; }
status_t BTimeSource::_Reserved_TimeSource_2(void *) { return 0; }
status_t BTimeSource::_Reserved_TimeSource_3(void *) { return 0; }
status_t BTimeSource::_Reserved_TimeSource_4(void *) { return 0; }
status_t BTimeSource::_Reserved_TimeSource_5(void *) { return 0; }

/* explicit */
BTimeSource::BTimeSource(media_node_id id)
	: BMediaNode("XXX fixme")
{
	UNIMPLEMENTED();
}


void
BTimeSource::FinishCreate()
{
	UNIMPLEMENTED();
}


status_t
BTimeSource::RemoveMe(BMediaNode *node)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BTimeSource::AddMe(BMediaNode *node)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BTimeSource::DirectStart(bigtime_t at)
{
	UNIMPLEMENTED();
}


void
BTimeSource::DirectStop(bigtime_t at,
						bool immediate)
{
	UNIMPLEMENTED();
}


void
BTimeSource::DirectSeek(bigtime_t to,
						bigtime_t at)
{
	UNIMPLEMENTED();
}


void
BTimeSource::DirectSetRunMode(run_mode mode)
{
	UNIMPLEMENTED();
}


