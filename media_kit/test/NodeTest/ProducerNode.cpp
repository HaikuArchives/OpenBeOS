#include "ProducerNode.h"
#include "misc.h"

ProducerNode::ProducerNode() : 
	BBufferProducer(B_MEDIA_RAW_AUDIO),
	BMediaEventLooper(),
	BMediaNode("ProducerNode")
{
	out("ProducerNode::ProducerNode\n");

	// initialize our preferred format object
	mPreferredFormat.type = B_MEDIA_RAW_AUDIO;
	mPreferredFormat.u.raw_audio.format = media_raw_audio_format::B_AUDIO_FLOAT;
	mPreferredFormat.u.raw_audio.channel_count = 1;
	mPreferredFormat.u.raw_audio.frame_rate = 44100;		// measured in Hertz
	mPreferredFormat.u.raw_audio.byte_order = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;

	// we'll use the consumer's preferred buffer size, if any
	mPreferredFormat.u.raw_audio.buffer_size = media_raw_audio_format::wildcard.buffer_size;

	// we're not connected yet
	mOutput.source = media_source::null;
	mOutput.destination = media_destination::null;
	mOutput.format = mPreferredFormat;

}

status_t 
ProducerNode::FormatSuggestionRequested(
				media_type type,
				int32 quality,
				media_format * format)
{
	out("ProducerNode::FormatSuggestionRequested\n");
	return B_OK;
}

status_t 
ProducerNode::FormatProposal(
				const media_source & output,
				media_format * format)
{
	out("ProducerNode::FormatProposal\n");
	return B_OK;
}

status_t 
ProducerNode::FormatChangeRequested(
				const media_source & source,
				const media_destination & destination,
				media_format * io_format,
				int32 * _deprecated_)
{
	out("ProducerNode::FormatChangeRequested\n");
	return B_OK;
}

status_t 
ProducerNode::GetNextOutput(	/* cookie starts as 0 */
				int32 * cookie,
				media_output * out_output)
{
	out("ProducerNode::GetNextOutput\n");
	if (++(*cookie) > 1)
		return B_BAD_INDEX;
		
	mOutput.node = Node();
	*out_output = mOutput;
	return B_OK;
}

status_t 
ProducerNode::DisposeOutputCookie(
				int32 cookie)
{
	out("ProducerNode::DisposeOutputCookie\n");
	return B_OK;
}

status_t 
ProducerNode::SetBufferGroup(
				const media_source & for_source,
				BBufferGroup * group)
{
	out("ProducerNode::SetBufferGroup\n");
	return B_OK;
}

status_t 
ProducerNode::PrepareToConnect(
				const media_source & what,
				const media_destination & where,
				media_format * format,
				media_source * out_source,
				char * out_name)
{
	out("ProducerNode::PrepareToConnect\n");
	return B_OK;
}

void
ProducerNode::Connect(
				status_t error, 
				const media_source & source,
				const media_destination & destination,
				const media_format & format,
				char * io_name)
{
	out("ProducerNode::Connect\n");
	return;
}

void
ProducerNode::Disconnect(
				const media_source & what,
				const media_destination & where)
{
	out("ProducerNode::Disconnect\n");
	return;
}

void
ProducerNode::LateNoticeReceived(
				const media_source & what,
				bigtime_t how_much,
				bigtime_t performance_time)
{
	out("ProducerNode::LateNoticeReceived\n");
	return;
}

void
ProducerNode::EnableOutput(
				const media_source & what,
				bool enabled,
				int32 * _deprecated_)
{
	out("ProducerNode::EnableOutput\n");
	return;
}

BMediaAddOn* 
ProducerNode::AddOn(int32 * internal_id) const
{
	out("ProducerNode::AddOn\n");
	return NULL;
}

void 
ProducerNode::HandleEvent(const media_timed_event *event,
						 bigtime_t lateness,
						 bool realTimeEvent)
{
	out("ProducerNode::HandleEvent\n");
}


status_t 
ProducerNode::HandleMessage(int32 message,const void *data, size_t size)
{
	out("ProducerNode::HandleMessage %lx\n",message);
	if (B_OK == BMediaEventLooper::HandleMessage(message,data,size))
		return B_OK;
	if (B_OK == BBufferProducer::HandleMessage(message,data,size))
		return B_OK;
	return BMediaNode::HandleMessage(message,data,size);
}
