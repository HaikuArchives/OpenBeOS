#include "ConsumerNode.h"
#include "misc.h"

ConsumerNode::ConsumerNode() : 
	BBufferConsumer(B_MEDIA_RAW_AUDIO),
	BMediaEventLooper(),
	BMediaNode("ConsumerNode")
{
	out("ConsumerNode::ConsumerNode\n");

	// initialize our preferred format object
	mPreferredFormat.type = B_MEDIA_RAW_AUDIO;
	mPreferredFormat.u.raw_audio.format = media_raw_audio_format::B_AUDIO_FLOAT;
	mPreferredFormat.u.raw_audio.channel_count = 1;
	mPreferredFormat.u.raw_audio.frame_rate = 44100;		// measured in Hertz
	mPreferredFormat.u.raw_audio.byte_order = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;

	// we'll use the consumer's preferred buffer size, if any
	mPreferredFormat.u.raw_audio.buffer_size = media_raw_audio_format::wildcard.buffer_size;

	// we're not connected yet
	mInput.destination = media_destination::null;
	mInput.source = media_source::null;
	mInput.format = mPreferredFormat;

}


status_t
ConsumerNode::AcceptFormat(
				const media_destination & dest,
				media_format * format)
{
	out("ConsumerNode::AcceptFormat\n");
	return B_OK;
}

status_t
ConsumerNode::GetNextInput(
				int32 * cookie,
				media_input * out_input)
{
	out("ConsumerNode::GetNextInput\n");
	if (++(*cookie) > 1)
		return B_BAD_INDEX;
		
	mInput.node = Node();
	*out_input = mInput;
	return B_OK;
}

void
ConsumerNode::DisposeInputCookie(
				int32 cookie)
{
	out("ConsumerNode::DisposeInputCookie\n");
	return;
}

void
ConsumerNode::BufferReceived(
				BBuffer * buffer)
{
	out("ConsumerNode::BufferReceived\n");
	return;
}

void
ConsumerNode::ProducerDataStatus(
				const media_destination & for_whom,
				int32 status,
				bigtime_t at_performance_time)
{
	out("ConsumerNode::ProducerDataStatus\n");
	return;
}

status_t
ConsumerNode::GetLatencyFor(
				const media_destination & for_whom,
				bigtime_t * out_latency,
				media_node_id * out_timesource)
{
	out("ConsumerNode::GetLatencyFor\n");
	return B_OK;
}

status_t
ConsumerNode::Connected(
				const media_source & producer,
				const media_destination & where,
				const media_format & with_format,
				media_input * out_input)
{
	out("ConsumerNode::Connected\n");
	return B_OK;
}

void
ConsumerNode::Disconnected(
				const media_source & producer,
				const media_destination & where)
{
	out("ConsumerNode::Disconnected\n");
	return;
}

status_t
ConsumerNode::FormatChanged(
				const media_source & producer,
				const media_destination & consumer, 
				int32 change_tag,
				const media_format & format)
{
	out("ConsumerNode::FormatChanged\n");
	return B_OK;
}

BMediaAddOn* 
ConsumerNode::AddOn(int32 * internal_id) const
{
	out("ConsumerNode::AddOn\n");
	return NULL;
}

void 
ConsumerNode::HandleEvent(const media_timed_event *event,
						 bigtime_t lateness,
						 bool realTimeEvent)
{
	out("ConsumerNode::HandleEvent\n");
}

status_t 
ConsumerNode::HandleMessage(int32 message,const void *data, size_t size)
{
	out("ConsumerNode::HandleMessage %lx\n",message);
	if (B_OK == BMediaEventLooper::HandleMessage(message,data,size))
		return B_OK;
	if (B_OK == BBufferConsumer::HandleMessage(message,data,size))
		return B_OK;
	return BMediaNode::HandleMessage(message,data,size);
}
