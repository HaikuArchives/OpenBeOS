/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: MediaNode.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaNode.h>
#include <TimeSource.h>
#include "debug.h"

/*************************************************************
 * media_node 
 *************************************************************/

// final & verfied
media_node::media_node()
	: node(-1),
	port(-1),
	kind(0)
{
	CALLED();
}

// final & verfied
media_node::~media_node()
{
	CALLED();
}

/*************************************************************
 * static media_node variables
 *************************************************************/

// final & verfied
media_node media_node::null;

/*************************************************************
 * media_input 
 *************************************************************/

// final
media_input::media_input()
{
	CALLED();
	name[0] = '\0';
}

// final
media_input::~media_input()
{
	CALLED();
}

/*************************************************************
 * media_output 
 *************************************************************/

// final
media_output::media_output()
{
	CALLED();
	name[0] = '\0';
}

// final
media_output::~media_output()
{
	CALLED();
}

/*************************************************************
 * live_node_info 
 *************************************************************/

// final & verfied
live_node_info::live_node_info()
	: hint_point(0.0f,0.0f)
{
	CALLED();
	name[0] = '\0';
}

// final & verfied
live_node_info::~live_node_info()
{
	CALLED();
}

/*************************************************************
 * protected BMediaNode
 *************************************************************/

/* virtual */
BMediaNode::~BMediaNode()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BMediaNode
 *************************************************************/

BMediaNode *
BMediaNode::Acquire()
{
	UNIMPLEMENTED();
	return NULL;
}


BMediaNode *
BMediaNode::Release()
{
	UNIMPLEMENTED();
	return NULL;
}


const char *
BMediaNode::Name() const
{
	UNIMPLEMENTED();
	return NULL;
}


media_node_id
BMediaNode::ID() const
{
	UNIMPLEMENTED();
	media_node_id dummy;

	return dummy;
}


uint64
BMediaNode::Kinds() const
{
	UNIMPLEMENTED();
	uint64 dummy;

	return dummy;
}


media_node
BMediaNode::Node() const
{
	UNIMPLEMENTED();
	media_node dummy;

	return dummy;
}


BMediaNode::run_mode
BMediaNode::RunMode() const
{
	UNIMPLEMENTED();
	run_mode dummy;

	return dummy;
}


BTimeSource *
BMediaNode::TimeSource() const
{
	CALLED();
	return _mTimeSource;
}


/* virtual */ port_id
BMediaNode::ControlPort() const
{
	UNIMPLEMENTED();
	port_id dummy;

	return dummy;
}


/*************************************************************
 * protected BMediaNode
 *************************************************************/

status_t
BMediaNode::ReportError(node_error what,
						const BMessage *info)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BMediaNode::NodeStopped(bigtime_t whenPerformance)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BMediaNode::TimerExpired(bigtime_t notifyPoint,
						 int32 cookie,
						 status_t error)
{
	UNIMPLEMENTED();
}

/* explicit */
BMediaNode::BMediaNode(const char *name)
{
	UNIMPLEMENTED();
}


status_t
BMediaNode::WaitForMessage(bigtime_t waitUntil,
						   uint32 flags,
						   void *_reserved_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ void
BMediaNode::Start(bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::Stop(bigtime_t performance_time,
				 bool immediate)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::Seek(bigtime_t media_time,
				 bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::SetRunMode(run_mode mode)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::TimeWarp(bigtime_t at_real_time,
					 bigtime_t to_performance_time)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::Preroll()
{
	UNIMPLEMENTED();
}


/* virtual */ void
BMediaNode::SetTimeSource(BTimeSource *time_source)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BMediaNode
 *************************************************************/

/* virtual */ status_t
BMediaNode::HandleMessage(int32 message,
						  const void *data,
						  size_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BMediaNode::HandleBadMessage(int32 code,
							 const void *buffer,
							 size_t size)
{
	UNIMPLEMENTED();
}


void
BMediaNode::AddNodeKind(uint64 kind)
{
	UNIMPLEMENTED();
}


void *
BMediaNode::operator new(size_t size)
{
	CALLED();
	return ::operator new(size);
}

void *
BMediaNode::operator new(size_t size,
						 const nothrow_t &) throw()
{
	CALLED();
	return ::operator new(size,nothrow);
}

void
BMediaNode::operator delete(void *ptr)
{
	CALLED();
	::operator delete(ptr);
}

void 
BMediaNode::operator delete(void * ptr, 
							const nothrow_t &) throw()
{
	CALLED();
	::operator delete(ptr,nothrow);
}

/*************************************************************
 * protected BMediaNode
 *************************************************************/

/* virtual */ status_t
BMediaNode::RequestCompleted(const media_request_info &info)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BMediaNode
 *************************************************************/

int32
BMediaNode::IncrementChangeTag()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


int32
BMediaNode::ChangeTag()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


int32
BMediaNode::MintChangeTag()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


status_t
BMediaNode::ApplyChangeTag(int32 previously_reserved)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * protected BMediaNode
 *************************************************************/

/* virtual */ status_t
BMediaNode::DeleteHook(BMediaNode *node)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ void
BMediaNode::NodeRegistered()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BMediaNode
 *************************************************************/

/* virtual */ status_t
BMediaNode::GetNodeAttributes(media_node_attribute *outAttributes,
							  size_t inMaxCount)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
BMediaNode::AddTimer(bigtime_t at_performance_time,
					 int32 cookie)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t BMediaNode::_Reserved_MediaNode_0(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_1(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_2(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_3(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_4(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_5(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_6(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_7(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_8(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_9(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_10(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_11(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_12(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_13(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_14(void *) { return 0; }
status_t BMediaNode::_Reserved_MediaNode_15(void *) { return 0; }

/*
private unimplemented
BMediaNode::BMediaNode()
BMediaNode::BMediaNode(const BMediaNode &clone)
BMediaNode &BMediaNode::operator=(const BMediaNode &clone)
*/

BMediaNode::BMediaNode(const char *name,
					   media_node_id id,
					   uint32 kinds)
{
	UNIMPLEMENTED();
}


void
BMediaNode::_inspect_classes()
{
	UNIMPLEMENTED();
}


void
BMediaNode::PStart(bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


void
BMediaNode::PStop(bigtime_t performance_time,
				  bool immediate)
{
	UNIMPLEMENTED();
}


void
BMediaNode::PSeek(bigtime_t media_time,
				  bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


void
BMediaNode::PSetRunMode(run_mode mode,
						bigtime_t recordDelay)
{
	UNIMPLEMENTED();
}


void
BMediaNode::PTimeWarp(bigtime_t at_real_time,
					  bigtime_t to_performance_time)
{
	UNIMPLEMENTED();
}


void
BMediaNode::PPreroll()
{
	UNIMPLEMENTED();
}


void
BMediaNode::PSetTimeSource(BTimeSource *time_source)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * protected BMediaNode
 *************************************************************/

/* static */ int32
BMediaNode::NewChangeTag()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


