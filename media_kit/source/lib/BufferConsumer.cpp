/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferConsumer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <BufferConsumer.h>
#include "debug.h"

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* virtual */
BBufferConsumer::~BBufferConsumer()
{
	UNIMPLEMENTED();
}


/*************************************************************
 * public BBufferConsumer
 *************************************************************/

media_type
BBufferConsumer::ConsumerType()
{
	UNIMPLEMENTED();
	media_type dummy;

	return dummy;
}


/* static */ status_t
BBufferConsumer::RegionToClipData(const BRegion *region,
								  int32 *format,
								  int32 *ioSize,
								  void *data)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* explicit */
BBufferConsumer::BBufferConsumer(media_type consumer_type)
	: BMediaNode("called by BBufferConsumer")
{
	UNIMPLEMENTED();
	
	AddNodeKind(B_BUFFER_CONSUMER);
}


/* static */ void
BBufferConsumer::NotifyLateProducer(const media_source &what_source,
									bigtime_t how_much,
									bigtime_t performance_time)
{
	UNIMPLEMENTED();
}


status_t
BBufferConsumer::SetVideoClippingFor(const media_source &output,
									 const media_destination &destination,
									 const int16 *shorts,
									 int32 short_count,
									 const media_video_display_info &display,
									 void *user_data,
									 int32 *change_tag,
									 void *_reserved_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::SetOutputEnabled(const media_source &source,
								  const media_destination &destination,
								  bool enabled,
								  void *user_data,
								  int32 *change_tag,
								  void *_reserved_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::RequestFormatChange(const media_source &source,
									 const media_destination &destination,
									 const media_format &to_format,
									 void *user_data,
									 int32 *change_tag,
									 void *_reserved_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::RequestAdditionalBuffer(const media_source &source,
										 BBuffer *prev_buffer,
										 void *_reserved)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::RequestAdditionalBuffer(const media_source &source,
										 bigtime_t start_time,
										 void *_reserved)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::SetOutputBuffersFor(const media_source &source,
									 const media_destination &destination,
									 BBufferGroup *group,
									 void *user_data,
									 int32 *change_tag,
									 bool will_reclaim,
									 void *_reserved_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::SendLatencyChange(const media_source &source,
								   const media_destination &destination,
								   bigtime_t my_new_latency,
								   uint32 flags)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* virtual */ status_t
BBufferConsumer::HandleMessage(int32 message,
							   const void *data,
							   size_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::SeekTagRequested(const media_destination &destination,
								  bigtime_t in_target_time,
								  uint32 in_flags,
								  media_seek_tag *out_seek_tag,
								  bigtime_t *out_tagged_time,
								  uint32 *out_flags)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BBufferConsumer
 *************************************************************/

/*
unimplemented:
BBufferConsumer::BBufferConsumer()
BBufferConsumer::BBufferConsumer(const BBufferConsumer &clone)
BBufferConsumer & BBufferConsumer::operator=(const BBufferConsumer &clone)
*/

/* deprecated function for R4 */
/* static */ status_t
BBufferConsumer::SetVideoClippingFor(const media_source &output,
									 const int16 *shorts,
									 int32 short_count,
									 const media_video_display_info &display,
									 int32 *change_tag)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* deprecated function for R4 */
/* static */ status_t
BBufferConsumer::RequestFormatChange(const media_source &source,
									 const media_destination &destination,
									 media_format *io_to_format,
									 int32 *change_tag)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* deprecated function for R4 */
/* static */ status_t
BBufferConsumer::SetOutputEnabled(const media_source &source,
								  bool enabled,
								  int32 *change_tag)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t BBufferConsumer::_Reserved_BufferConsumer_0(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_1(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_2(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_3(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_4(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_5(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_6(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_7(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_8(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_9(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_10(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_11(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_12(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_13(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_14(void *) { return 0; }
status_t BBufferConsumer::_Reserved_BufferConsumer_15(void *) { return 0; }

