/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferProducer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <BufferProducer.h>
#include "debug.h"
#include "../server/headers/ServerInterface.h"

/*************************************************************
 * protected BBufferProducer
 *************************************************************/

BBufferProducer::~BBufferProducer()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BBufferProducer
 *************************************************************/

/* static */ status_t
BBufferProducer::ClipDataToRegion(int32 format,
								  int32 size,
								  const void *data,
								  BRegion *region)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

media_type
BBufferProducer::ProducerType()
{
	UNIMPLEMENTED();
	media_type dummy;

	return dummy;
}

/*************************************************************
 * protected BBufferProducer
 *************************************************************/

/* explicit */
BBufferProducer::BBufferProducer(media_type producer_type)
	: BMediaNode("called by BBufferProducer")
{
	UNIMPLEMENTED();

	AddNodeKind(B_BUFFER_PRODUCER);
}


status_t
BBufferProducer::VideoClippingChanged(const media_source &for_source,
									  int16 num_shorts,
									  int16 *clip_data,
									  const media_video_display_info &display,
									  int32 *_deprecated_)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::GetLatency(bigtime_t *out_lantency)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::SetPlayRate(int32 numer,
							 int32 denom)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

		
status_t
BBufferProducer::HandleMessage(int32 message,
							   const void *data,
							   size_t size)
{
	CALLED();
	switch (message) {
	
		case PRODUCER_ADDITIONAL_BUFFER_REQUESTED:
		{
			const xfer_producer_additional_buffer_requested *data = (const xfer_producer_additional_buffer_requested *)data;
			AdditionalBufferRequested(data->source, data->prev_buffer, data->prev_time, data->has_seek_tag ? &data->prev_tag : NULL);
			return B_OK;
		}
	
		case PRODUCER_LATENCY_CHANGE:
		{
			const xfer_producer_latency_change *data = (const xfer_producer_latency_change *)data;
			LatencyChanged(data->source, data->destination, data->latency, data->flags);
			return B_OK;
		}

		case PRODUCER_LATE_NOTICE:
		{
			const xfer_producer_late_notice *data = (const xfer_producer_late_notice *)data;
			LateNoticeReceived(data->source, data->how_much, data->performance_time);
			return B_OK;
		}

		case PRODUCER_OUTPUT_ENABLE:
		{
			const xfer_producer_output_enable *data = (const xfer_producer_output_enable *)data;
			xfer_node_request_completed reply;
			EnableOutput(data->source, data->enabled, NULL);
			if (data->destination == media_destination::null)
				return B_OK;
			reply.info.what = media_request_info::B_SET_OUTPUT_ENABLED;
			reply.info.change_tag = data->change_tag;
			reply.info.status = B_OK;
			//reply.info.cookie
			reply.info.user_data = data->user_data;
			reply.info.source = data->source;
			reply.info.destination = data->destination;
			//reply.info.format
			write_port(data->destination.port, NODE_REQUEST_COMPLETED, &reply, sizeof(reply));
			return B_OK;
		}
				
	};
	return B_ERROR;
}


void
BBufferProducer::AdditionalBufferRequested(const media_source &source,
										   media_buffer_id prev_buffer,
										   bigtime_t prev_time,
										   const media_seek_tag *prev_tag)
{
	CALLED();
	// may be implemented by derived classes
}


void
BBufferProducer::LatencyChanged(const media_source &source,
								const media_destination &destination,
								bigtime_t new_latency,
								uint32 flags)
{
	CALLED();
	// may be implemented by derived classes
}


status_t
BBufferProducer::SendBuffer(BBuffer *buffer,
							const media_destination &destination)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::SendDataStatus(int32 status,
								const media_destination &destination,
								bigtime_t at_time)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::ProposeFormatChange(media_format *format,
									 const media_destination &for_destination)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::ChangeFormat(const media_source &for_source,
							  const media_destination &for_destination,
							  media_format *format)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::FindLatencyFor(const media_destination &for_destination,
								bigtime_t *out_latency,
								media_node_id *out_timesource)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::FindSeekTag(const media_destination &for_destination,
							 bigtime_t in_target_time,
							 media_seek_tag *out_tag,
							 bigtime_t *out_tagged_time,
							 uint32 *out_flags,
							 uint32 in_flags)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BBufferProducer::SetInitialLatency(bigtime_t inInitialLatency,
								   uint32 flags)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * private BBufferProducer
 *************************************************************/

/*
private unimplemented
BBufferProducer::BBufferProducer()
BBufferProducer::BBufferProducer(const BBufferProducer &clone)
BBufferProducer & BBufferProducer::operator=(const BBufferProducer &clone)
*/

status_t BBufferProducer::_Reserved_BufferProducer_0(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_1(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_2(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_3(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_4(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_5(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_6(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_7(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_8(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_9(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_10(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_11(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_12(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_13(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_14(void *) { return B_ERROR; }
status_t BBufferProducer::_Reserved_BufferProducer_15(void *) { return B_ERROR; }


status_t
BBufferProducer::clip_shorts_to_region(int16 *data,
									   int count,
									   BRegion *output)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::clip_region_to_shorts(const BRegion *input,
									   int16 *data,
									   int max_count,
									   int *out_count)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BBufferProducer::SendRequestResult(const media_request_info &result,
								   port_id port,
								   bool sync)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BBufferProducer::PSetRecordDelay(bigtime_t inDelay)
{
	UNIMPLEMENTED();
}


