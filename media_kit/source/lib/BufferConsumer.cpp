/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: BufferConsumer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <BufferConsumer.h>
#include <Buffer.h>
#include "debug.h"
#include "../server/headers/ServerInterface.h"
#include "BufferIdCache.h"

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* virtual */
BBufferConsumer::~BBufferConsumer()
{
	CALLED();
	delete fBufferCache;
}


/*************************************************************
 * public BBufferConsumer
 *************************************************************/

media_type
BBufferConsumer::ConsumerType()
{
	CALLED();
	return fConsumerType;
}


/* static */ status_t
BBufferConsumer::RegionToClipData(const BRegion *region,
								  int32 *format,
								  int32 *ioSize,
								  void *data)
{
	UNIMPLEMENTED();

	return B_ERROR;
}

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* explicit */
BBufferConsumer::BBufferConsumer(media_type consumer_type) :
	BMediaNode("called by BBufferConsumer"),
	fConsumerType(consumer_type),
	fBufferCache(new _buffer_id_cache)
{
	CALLED();
	
	AddNodeKind(B_BUFFER_CONSUMER);
}


/* static */ void
BBufferConsumer::NotifyLateProducer(const media_source &what_source,
									bigtime_t how_much,
									bigtime_t performance_time)
{
	CALLED();
	if (what_source == media_source::null)
		return;

	xfer_producer_late_notice data;
	data.source = what_source;
	data.how_much = how_much;
	data.performance_time = performance_time;
	
	write_port(what_source.port, PRODUCER_LATE_NOTICE, &data, sizeof(data));
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
	CALLED();
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
	CALLED();
	if (destination == media_destination::null)
		return B_MEDIA_BAD_DESTINATION;
	if (source == media_source::null)
		return B_MEDIA_BAD_SOURCE;

	xfer_producer_output_enable data;
	
	data.source = source;
	data.destination = destination;
	data.enabled = enabled;
	data.user_data = user_data;
	data.change_tag = NewChangeTag();
	if (change_tag != NULL)
		*change_tag = data.change_tag;
	
	return write_port(source.port, PRODUCER_OUTPUT_ENABLE, &data, sizeof(data));
}


status_t
BBufferConsumer::RequestFormatChange(const media_source &source,
									 const media_destination &destination,
									 const media_format &to_format,
									 void *user_data,
									 int32 *change_tag,
									 void *_reserved_)
{
	CALLED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::RequestAdditionalBuffer(const media_source &source,
										 BBuffer *prev_buffer,
										 void *_reserved)
{
	CALLED();
	if (source == media_source::null)
		return B_MEDIA_BAD_SOURCE;

	xfer_producer_additional_buffer_requested data;
	
	data.source = source;
	data.prev_buffer = prev_buffer->ID();
	data.prev_time = 0;
	data.has_seek_tag = false;
	//data.prev_tag = 
	
	return write_port(source.port, PRODUCER_ADDITIONAL_BUFFER_REQUESTED, &data, sizeof(data));
}


status_t
BBufferConsumer::RequestAdditionalBuffer(const media_source &source,
										 bigtime_t start_time,
										 void *_reserved)
{
	CALLED();
	if (source == media_source::null)
		return B_MEDIA_BAD_SOURCE;

	xfer_producer_additional_buffer_requested data;
	
	data.source = source;
	data.prev_buffer = 0;
	data.prev_time = start_time;
	data.has_seek_tag = false;
	//data.prev_tag = 
	
	return write_port(source.port, PRODUCER_ADDITIONAL_BUFFER_REQUESTED, &data, sizeof(data));
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
	CALLED();
	status_t dummy;

	return dummy;
}


status_t
BBufferConsumer::SendLatencyChange(const media_source &source,
								   const media_destination &destination,
								   bigtime_t my_new_latency,
								   uint32 flags)
{
	CALLED();
	if (destination == media_destination::null)
		return B_MEDIA_BAD_DESTINATION;
	if (source == media_source::null)
		return B_MEDIA_BAD_SOURCE;

	xfer_producer_latency_change data;
	
	data.source = source;
	data.destination = destination;
	data.latency = my_new_latency;
	data.flags = flags;
	
	return write_port(source.port, PRODUCER_LATENCY_CHANGE, &data, sizeof(data));
}

/*************************************************************
 * protected BBufferConsumer
 *************************************************************/

/* virtual */ status_t
BBufferConsumer::HandleMessage(int32 message,
							   const void *data,
							   size_t size)
{
	CALLED();
	switch (message) {
		case CONSUMER_ACCEPT_FORMAT:
		{
			const xfer_producer_accept_format *data = (const xfer_producer_accept_format *)data;
			xfer_producer_accept_format_reply reply;
			reply.format = data->format;
			reply.result = AcceptFormat(data->dest, &reply.format);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}

		case CONSUMER_GET_NEXT_INPUT:
		{
			const xfer_producer_get_next_input *data = (const xfer_producer_get_next_input *)data;
			xfer_producer_get_next_input_reply reply;
			reply.cookie = data->cookie;
			reply.result = GetNextInput(&reply.cookie, &reply.input);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}

		case CONSUMER_DISPOSE_INPUT_COOKIE:
		{
			const xfer_producer_dispose_input_cookie *data = (const xfer_producer_dispose_input_cookie *)data;
			DisposeInputCookie(data->cookie);
			return B_OK;
		}

		case CONSUMER_BUFFER_RECEIVED:
		{
			const xfer_producer_buffer_received *data = (const xfer_producer_buffer_received *)data;
			BBuffer *buffer;
			buffer = fBufferCache->GetBuffer(data->buffer);
			buffer->SetHeader(&data->header);
			BufferReceived(buffer);
			return B_OK;
		}

		case CONSUMER_PRODUCER_DATA_STATUS:
		{
			const xfer_producer_data_status *data = (const xfer_producer_data_status *)data;
			ProducerDataStatus(data->for_whom, data->status, data->at_performance_time);
			return B_OK;
		}

		case CONSUMER_GET_LATENCY_FOR:
		{
			const xfer_producer_get_latency_for *data = (const xfer_producer_get_latency_for *)data;
			xfer_producer_get_latency_for_reply reply;
			reply.result = GetLatencyFor(data->for_whom, &reply.latency, &reply.timesource);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}

		case CONSUMER_CONNECTED:
		{
			const xfer_producer_connected *data = (const xfer_producer_connected *)data;
			xfer_producer_connected_reply reply;
			reply.result = Connected(data->producer, data->where, data->with_format, &reply.input);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}
				
		case CONSUMER_DISCONNECTED:
		{
			const xfer_producer_disconnected *data = (const xfer_producer_disconnected *)data;
			Disconnected(data->producer, data->where);
			return B_OK;
		}

		case CONSUMER_FORMAT_CHANGED:
		{
			const xfer_producer_format_changed *data = (const xfer_producer_format_changed *)data;
			xfer_producer_format_changed_reply reply;
			reply.result = FormatChanged(data->producer, data->consumer, data->change_tag, data->format);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}

		case CONSUMER_SEEK_TAG_REQUESTED:
		{
			const xfer_producer_seek_tag_requested *data = (const xfer_producer_seek_tag_requested *)data;
			xfer_producer_seek_tag_requested_reply reply;
			reply.result = SeekTagRequested(data->destination, data->target_time, data->flags, &reply.seek_tag, &reply.tagged_time, &reply.flags);
			write_port(data->reply_port, 0, &reply, sizeof(reply));
			return B_OK;
		}

	};
	return B_ERROR;
}

status_t
BBufferConsumer::SeekTagRequested(const media_destination &destination,
								  bigtime_t in_target_time,
								  uint32 in_flags,
								  media_seek_tag *out_seek_tag,
								  bigtime_t *out_tagged_time,
								  uint32 *out_flags)
{
	CALLED();
	// may be implemented by derived classes
	return B_ERROR;
}

/*************************************************************
 * private BBufferConsumer
 *************************************************************/

/*
CALLED:
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
	CALLED();
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
	CALLED();
	status_t dummy;

	return dummy;
}


/* deprecated function for R4 */
/* static */ status_t
BBufferConsumer::SetOutputEnabled(const media_source &source,
								  bool enabled,
								  int32 *change_tag)
{
	CALLED();
	if (source == media_source::null)
		return B_MEDIA_BAD_SOURCE;

	xfer_producer_output_enable data;
	
	data.source = source;
	data.destination = media_destination::null;
	data.enabled = enabled;
	data.user_data = 0;
	data.change_tag = NewChangeTag();
	if (change_tag != NULL)
		*change_tag = data.change_tag;
	
	return write_port(source.port, PRODUCER_OUTPUT_ENABLE, &data, sizeof(data));
}


status_t BBufferConsumer::_Reserved_BufferConsumer_0(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_1(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_2(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_3(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_4(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_5(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_6(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_7(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_8(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_9(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_10(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_11(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_12(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_13(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_14(void *) { return B_ERROR; }
status_t BBufferConsumer::_Reserved_BufferConsumer_15(void *) { return B_ERROR; }

