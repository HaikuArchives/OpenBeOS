#include <BufferProducer.h>
#include <MediaEventLooper.h>

class ProducerNode : public virtual BBufferProducer, BMediaEventLooper
{
public:
	ProducerNode();

protected:
	/* functionality of BBufferProducer */
virtual	status_t FormatSuggestionRequested(
				media_type type,
				int32 quality,
				media_format * format);
virtual	status_t FormatProposal(
				const media_source & output,
				media_format * format);
virtual	status_t FormatChangeRequested(
				const media_source & source,
				const media_destination & destination,
				media_format * io_format,
				int32 * _deprecated_);
virtual	status_t GetNextOutput(	/* cookie starts as 0 */
				int32 * cookie,
				media_output * out_output);
virtual	status_t DisposeOutputCookie(
				int32 cookie);
virtual	status_t SetBufferGroup(
				const media_source & for_source,
				BBufferGroup * group);
	/* Iterates over all outputs and maxes the latency found */
virtual	status_t PrepareToConnect(
				const media_source & what,
				const media_destination & where,
				media_format * format,
				media_source * out_source,
				char * out_name);
virtual	void Connect(
				status_t error, 
				const media_source & source,
				const media_destination & destination,
				const media_format & format,
				char * io_name);
virtual	void Disconnect(
				const media_source & what,
				const media_destination & where);
virtual	void LateNoticeReceived(
				const media_source & what,
				bigtime_t how_much,
				bigtime_t performance_time);
virtual	void EnableOutput(
				const media_source & what,
				bool enabled,
				int32 * _deprecated_);

virtual status_t HandleMessage(int32 message,
				const void *data, size_t size);

/* from BMediaNode */
virtual	BMediaAddOn* AddOn(
				int32 * internal_id) const;

/* from BMediaEventLooper */
virtual void HandleEvent(const media_timed_event *event,
						 bigtime_t lateness,
						 bool realTimeEvent = false);

	media_output mOutput;
	media_format mPreferredFormat;
};

