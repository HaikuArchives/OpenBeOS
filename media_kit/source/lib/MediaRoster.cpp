/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: MediaRoster.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaRoster.h>
#include <Locker.h>
#include <Message.h>
#include <Messenger.h>
#include <StopWatch.h>
#include <OS.h>
#include "debug.h"
#include "PortPool.h"
#include "../server/headers/ServerInterface.h"

static BMessenger *ServerMessenger = 0;
static team_id team;

// the BMediaRoster destructor is private,
// but _DefaultDeleter is a friend class of
// the BMediaRoster an thus can delete it
class _DefaultDeleter
{
public:
	void DeleteMediaRoster() 
	{ 
		delete BMediaRoster::_sDefault; 
	}
};

class RosterSingleton
{
public:
	RosterSingleton()
	{
		thread_info info;
		get_thread_info(find_thread(NULL), &info);
		team = info.team;
		ServerMessenger = new BMessenger(NEW_MEDIA_SERVER_SIGNATURE);
	}
	~RosterSingleton()
	{
		_DefaultDeleter deleter; 
		deleter.DeleteMediaRoster();
		delete ServerMessenger;
	}
};

RosterSingleton singleton;

namespace MediaKitPrivate 
{

status_t QueryServer(BMessage *query, BMessage *reply)
{
	status_t status;
	status = ServerMessenger->SendMessage(query,reply);
	if (status != B_OK || reply->what != B_OK) {
		TRACE("QueryServer failed! status = 0x%08lx\n",status);
		TRACE("Query:\n");
		query->PrintToStream();
		TRACE("Reply:\n");
		reply->PrintToStream();
	}
	return status;
}

};

/*************************************************************
 * public BMediaRoster
 *************************************************************/

status_t 
BMediaRoster::GetVideoInput(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

		
status_t 
BMediaRoster::GetAudioInput(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetVideoOutput(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetAudioMixer(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetAudioOutput(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetAudioOutput(media_node * out_node,
							 int32 * out_input_id,
							 BString * out_input_name)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

							 
status_t 
BMediaRoster::GetTimeSource(media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetVideoInput(const media_node & producer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetVideoInput(const dormant_node_info & producer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetAudioInput(const media_node & producer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetAudioInput(const dormant_node_info & producer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetVideoOutput(const media_node & consumer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetVideoOutput(const dormant_node_info & consumer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetAudioOutput(const media_node & consumer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetAudioOutput(const media_input & input_to_output)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetAudioOutput(const dormant_node_info & consumer)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetNodeFor(media_node_id node,
						 media_node * clone)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetSystemTimeSource(media_node * clone)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::ReleaseNode(const media_node & node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}



BTimeSource *
BMediaRoster::MakeTimeSourceFor(const media_node & for_node)
{
	UNIMPLEMENTED();
	return 0;
}


status_t 
BMediaRoster::Connect(const media_source & from,
					  const media_destination & to,
					  media_format * io_format,
					  media_output * out_output,
					  media_input * out_input)
{
	CALLED();
	if (io_format == NULL || out_output == NULL || out_input == NULL)
		return B_BAD_VALUE;
	if (from == media_source::null)
		return B_MEDIA_BAD_SOURCE;
	if (to == media_destination::null)
		return B_MEDIA_BAD_DESTINATION;

	xfer_producer_format_proposal msg1;
	xfer_producer_format_proposal_reply reply1;
	xfer_consumer_accept_format msg2;
	xfer_consumer_accept_format_reply reply2;
	xfer_producer_prepare_to_connect msg3;
	xfer_producer_prepare_to_connect_reply reply3;
	xfer_consumer_connected msg4;
	xfer_consumer_connected_reply reply4;
	xfer_producer_connect msg5;
	xfer_producer_connect_reply reply5;
	status_t rv;	
	port_id port;
	int32 code;

	port = _PortPool->GetPort();

	// BBufferProducer::FormatProposal
	msg1.output = from;
	msg1.format = *io_format;
	msg1.reply_port = port;
	rv = write_port(from.port, PRODUCER_FORMAT_PROPOSAL, &msg1, sizeof(msg1));
	if (rv != B_OK) 
		goto failed;
	rv = read_port(port, &code, &reply1, sizeof(reply1));
	if (rv < B_OK)
		goto failed;
	if (reply1.result != B_OK) {
		rv = reply1.result;
		goto failed;
	}

	// BBufferConsumer::AcceptFormat
	msg2.dest = to;
	msg2.format = *io_format;
	msg2.reply_port = port;
	rv = write_port(to.port, CONSUMER_ACCEPT_FORMAT, &msg2, sizeof(msg2));
	if (rv != B_OK) 
		goto failed;
	rv = read_port(port, &code, &reply2, sizeof(reply2));
	if (rv < B_OK)
		goto failed;
	if (reply2.result != B_OK) {
		rv = reply2.result;
		goto failed;
	}
	*io_format = reply2.format;
	
	// BBufferProducer::PrepareToConnect
	msg3.source = from;
	msg3.destination = to;
	msg3.format = *io_format;
	msg3.reply_port = port;
	rv = write_port(from.port, PRODUCER_PREPARE_TO_CONNECT, &msg3, sizeof(msg3));
	if (rv != B_OK) 
		goto failed;
	rv = read_port(port, &code, &reply3, sizeof(reply3));
	if (rv < B_OK)
		goto failed;
	if (reply3.result != B_OK) {
		rv = reply3.result;
		goto failed;
	}
	*io_format = reply3.format;
	//reply3.out_source;
	//reply3.name;

	// BBufferConsumer::Connected
	msg4.producer = reply3.out_source;
	msg4.where = to;
	msg4.with_format = *io_format;
	msg4.reply_port = port;
	rv = write_port(to.port, CONSUMER_CONNECTED, &msg4, sizeof(msg4));
	if (rv != B_OK) 
		goto failed;
	rv = read_port(port, &code, &reply4, sizeof(reply4));
	if (rv < B_OK)
		goto failed;
	if (reply4.result != B_OK) {
		rv = reply4.result;
		goto failed;
	}
	// reply4.input;

	// BBufferProducer::Connect
	msg5.error = B_OK;
	msg5.source = reply3.out_source;
	msg5.destination = to;
	msg5.format = *io_format;
	msg5.name[0] = 0;
	msg5.reply_port = port;

	rv = write_port(from.port, PRODUCER_CONNECT, &msg5, sizeof(msg5));
	if (rv != B_OK) 
		goto failed;
	rv = read_port(port, &code, &reply5, sizeof(reply5));
	if (rv < B_OK)
		goto failed;
	
//	out_output->node =
	out_output->source = reply3.out_source;
	out_output->destination = to;//reply4.input;
	out_output->format = *io_format;
	strcpy(out_output->name,reply5.name);

//	out_input->node
	out_input->source = reply3.out_source;
	out_input->destination = to;//reply4.input;
	out_input->format = *io_format;
	strcpy(out_input->name,reply3.name);

	_PortPool->PutPort(port);
	return B_OK;

failed:
	_PortPool->PutPort(port);
	return rv;
}


status_t 
BMediaRoster::Connect(const media_source & from,
					  const media_destination & to,
					  media_format * io_format,
					  media_output * out_output,
					  media_input * out_input,
					  uint32 in_flags,
					  void * _reserved)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

					  
status_t 
BMediaRoster::Disconnect(media_node_id source_node,
						 const media_source & source,
						 media_node_id destination_node,
						 const media_destination & destination)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::StartNode(const media_node & node,
						bigtime_t at_performance_time)
{
	CALLED();
	if (node.node == 0)
		return B_MEDIA_BAD_NODE;

	xfer_node_start msg;
	msg.performance_time = at_performance_time;
	
	return write_port(node.port, NODE_START, &msg, sizeof(msg));
}


status_t 
BMediaRoster::StopNode(const media_node & node,
					   bigtime_t at_performance_time,
					   bool immediate)
{
	CALLED();
	if (node.node == 0)
		return B_MEDIA_BAD_NODE;

	xfer_node_stop msg;
	msg.performance_time = at_performance_time;
	msg.immediate = immediate;
	
	return write_port(node.port, NODE_STOP, &msg, sizeof(msg));
}

					   
status_t 
BMediaRoster::SeekNode(const media_node & node,
					   bigtime_t to_media_time,
					   bigtime_t at_performance_time)
{
	CALLED();
	if (node.node == 0)
		return B_MEDIA_BAD_NODE;

	xfer_node_seek msg;
	msg.media_time = to_media_time;
	msg.performance_time = at_performance_time;
	
	return write_port(node.port, NODE_SEEK, &msg, sizeof(msg));
}


status_t 
BMediaRoster::StartTimeSource(const media_node & node,
							  bigtime_t at_real_time)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

							  
status_t 
BMediaRoster::StopTimeSource(const media_node & node,
							 bigtime_t at_real_time,
							 bool immediate)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

							 
status_t 
BMediaRoster::SeekTimeSource(const media_node & node,
							 bigtime_t to_performance_time,
							 bigtime_t at_real_time)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SyncToNode(const media_node & node,
						 bigtime_t at_time,
						 bigtime_t timeout)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

						 
status_t 
BMediaRoster::SetRunModeNode(const media_node & node,
							 BMediaNode::run_mode mode)
{
	CALLED();
	if (node.node == 0)
		return B_MEDIA_BAD_NODE;

	xfer_node_set_run_mode msg;
	msg.mode = mode;
	
	return write_port(node.port, NODE_SET_RUN_MODE, &msg, sizeof(msg));
}

							 
status_t 
BMediaRoster::PrerollNode(const media_node & node)
{
	CALLED();
	if (node.node == 0)
		return B_MEDIA_BAD_NODE;

	char dummy;
	return write_port(node.port, NODE_PREROLL, &dummy, sizeof(dummy));
}


status_t 
BMediaRoster::RollNode(const media_node & node, 
					   bigtime_t startPerformance,
					   bigtime_t stopPerformance,
					   bigtime_t atMediaTime)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetProducerRunModeDelay(const media_node & node,
									  bigtime_t delay,
									  BMediaNode::run_mode mode)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetProducerRate(const media_node & producer,
							  int32 numer,
							  int32 denom)
{
	UNIMPLEMENTED();
	return B_ERROR;
}



/* Nodes will have available inputs/outputs as long as they are capable */
/* of accepting more connections. The node may create an additional */
/* output or input as the currently available is taken into usage. */
status_t 
BMediaRoster::GetLiveNodeInfo(const media_node & node,
							  live_node_info * out_live_info)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetLiveNodes(live_node_info * out_live_nodes,
						   int32 * io_total_count,
						   const media_format * has_input,
						   const media_format * has_output,
						   const char * name,
						   uint64 node_kinds)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetFreeInputsFor(const media_node & node,
							   media_input * out_free_inputs,
							   int32 buf_num_inputs,
							   int32 * out_total_count,
							   media_type filter_type)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

							   
status_t 
BMediaRoster::GetConnectedInputsFor(const media_node & node,
									media_input * out_active_inputs,
									int32 buf_num_inputs,
									int32 * out_total_count)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

									
status_t 
BMediaRoster::GetAllInputsFor(const media_node & node,
							  media_input * out_inputs,
							  int32 buf_num_inputs,
							  int32 * out_total_count)
{
	CALLED();
	if (node.node == 0 || (node.kind & B_BUFFER_CONSUMER) == 0)
		return B_MEDIA_BAD_NODE;
	if (out_inputs == NULL || out_total_count == NULL)
		return B_BAD_VALUE;
		
	status_t rv;
	status_t rv2;
	port_id port;
	int32 code;
	int32 cookie;
		
	port = _PortPool->GetPort();
	*out_total_count = 0;
	cookie = 0;
	for (int32 i = 0; i < buf_num_inputs; i++) {
		xfer_consumer_get_next_input msg;		
		xfer_consumer_get_next_input_reply reply;
		msg.cookie = cookie;
		msg.reply_port = port;
		rv = write_port(node.port, CONSUMER_GET_NEXT_INPUT, &msg, sizeof(msg));
		if (rv != B_OK)
			break;
		rv = read_port(msg.reply_port, &code, &reply, sizeof(reply));
		if (rv < B_OK || reply.result != B_OK)
			break;
		*out_total_count += 1;
		out_inputs[i] = reply.input;
		cookie = reply.cookie;
	}
	_PortPool->PutPort(port);
	
	xfer_consumer_dispose_input_cookie msg2;
	msg2.cookie = cookie;
	rv2 = write_port(node.port, CONSUMER_DISPOSE_INPUT_COOKIE, &msg2, sizeof(msg2));

	return (rv != B_OK) ? rv : rv2;
}

							  
status_t 
BMediaRoster::GetFreeOutputsFor(const media_node & node,
								media_output * out_free_outputs,
								int32 buf_num_outputs,
								int32 * out_total_count,
								media_type filter_type)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

								
status_t 
BMediaRoster::GetConnectedOutputsFor(const media_node & node,
									 media_output * out_active_outputs,
									 int32 buf_num_outputs,
									 int32 * out_total_count)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetAllOutputsFor(const media_node & node,
							   media_output * out_outputs,
							   int32 buf_num_outputs,
							   int32 * out_total_count)
{
	CALLED();
TRACE("0\n");		
	if (node.node == 0 || (node.kind & B_BUFFER_PRODUCER) == 0)
		return B_MEDIA_BAD_NODE;
	if (out_outputs == NULL || out_total_count == NULL)
		return B_BAD_VALUE;

	status_t rv;
	status_t rv2;
	port_id port;
	int32 code;
	int32 cookie;
		
	port = _PortPool->GetPort();
	*out_total_count = 0;
	cookie = 0;
	for (int32 i = 0; i < buf_num_outputs; i++) {
		xfer_producer_get_next_output msg;		
		xfer_producer_get_next_output_reply reply;
		msg.cookie = cookie;
		msg.reply_port = port;
		rv = write_port(node.port, PRODUCER_GET_NEXT_OUTPUT, &msg, sizeof(msg));
		if (rv != B_OK)
			break;
		rv = read_port(msg.reply_port, &code, &reply, sizeof(reply));
		if (rv < B_OK || reply.result != B_OK)
			break;
		*out_total_count += 1;
		out_outputs[i] = reply.output;
		cookie = reply.cookie;
	}
	_PortPool->PutPort(port);
	
	xfer_producer_dispose_output_cookie msg2;
	msg2.cookie = cookie;
	rv2 = write_port(node.port, PRODUCER_DISPOSE_OUTPUT_COOKIE, &msg2, sizeof(msg2));

	return (rv != B_OK) ? rv : rv2;
}


status_t 
BMediaRoster::StartWatching(const BMessenger & where)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::StartWatching(const BMessenger & where,
							int32 notificationType)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::StartWatching(const BMessenger & where,
							const media_node & node,
							int32 notificationType)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

							
status_t 
BMediaRoster::StopWatching(const BMessenger & where)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::StopWatching(const BMessenger & where,
						   int32 notificationType)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

						   
status_t 
BMediaRoster::StopWatching(const BMessenger & where,
						   const media_node & node,
						   int32 notificationType)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::RegisterNode(BMediaNode * node)
{
	CALLED();
	if (node == NULL)
		return B_BAD_VALUE;

	xfer_node_registered msg;
	msg.node_id = 1;

	return node->HandleMessage(NODE_REGISTERED,&msg,sizeof(msg));
}


status_t 
BMediaRoster::UnregisterNode(BMediaNode * node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


//	thread safe for multiple calls to Roster()
/* static */ BMediaRoster * 
BMediaRoster::Roster(status_t* out_error)
{
	CALLED();
	static BLocker locker("BMediaRoster::Roster locker");
	locker.Lock();
	if (_sDefault == NULL) {
		_sDefault = new BMediaRoster();
		if (out_error != NULL)
			*out_error = B_OK;
	} else {
		if (out_error != NULL)
			*out_error = B_OK;
	}
	locker.Unlock();
	return _sDefault;
}


//	won't create it if there isn't one	
//	not thread safe if you call Roster() at the same time
/* static */ BMediaRoster * 
BMediaRoster::CurrentRoster()			
{
	CALLED();
	return _sDefault;
}

												
status_t 
BMediaRoster::SetTimeSourceFor(media_node_id node,
							   media_node_id time_source)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetParameterWebFor(const media_node & node, 
								 BParameterWeb ** out_web)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

								 
status_t 
BMediaRoster::StartControlPanel(const media_node & node,
								BMessenger * out_messenger)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetDormantNodes(dormant_node_info * out_info,
							  int32 * io_count,
							  const media_format * has_input,
							  const media_format * has_output,
							  const char * name,
							  uint64 require_kinds,
							  uint64 deny_kinds)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::InstantiateDormantNode(const dormant_node_info & in_info,
									 media_node * out_node,
									 uint32 flags /* currently B_FLAVOR_IS_GLOBAL or B_FLAVOR_IS_LOCAL */ )
{
	UNIMPLEMENTED();
	return B_ERROR;
}

									 
status_t 
BMediaRoster::InstantiateDormantNode(const dormant_node_info & in_info,
									 media_node * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetDormantNodeFor(const media_node & node,
								dormant_node_info * out_info)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetDormantFlavorInfoFor(const dormant_node_info & in_dormant,
									  dormant_flavor_info * out_flavor)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetLatencyFor(const media_node & producer,
							bigtime_t * out_latency)
{
	UNIMPLEMENTED();
	*out_latency = 0;
	return B_ERROR;
}


status_t 
BMediaRoster::GetInitialLatencyFor(const media_node & producer,
								   bigtime_t * out_latency,
								   uint32 * out_flags)
{
	UNIMPLEMENTED();
	*out_latency = 0;
	*out_flags = 0;
	return B_ERROR;
}


status_t 
BMediaRoster::GetStartLatencyFor(const media_node & time_source,
								 bigtime_t * out_latency)
{
	UNIMPLEMENTED();
	*out_latency = 0;
	return B_ERROR;
}


status_t 
BMediaRoster::GetFileFormatsFor(const media_node & file_interface, 
								media_file_format * out_formats,
								int32 * io_num_infos)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SetRefFor(const media_node & file_interface,
						const entry_ref & file,
						bool create_and_truncate,
						bigtime_t * out_length)	/* if create is false */
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetRefFor(const media_node & node,
						entry_ref * out_file,
						BMimeType * mime_type)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::SniffRefFor(const media_node & file_interface,
						  const entry_ref & file,
						  BMimeType * mime_type,
						  float * out_capability)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


/* This is the generic "here's a file, now can someone please play it" interface */
status_t 
BMediaRoster::SniffRef(const entry_ref & file,
					   uint64 require_node_kinds,		/* if you need an EntityInterface or BufferConsumer or something */
					   dormant_node_info * out_node,
					   BMimeType * mime_type)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetDormantNodeForType(const BMimeType & type,
									uint64 require_node_kinds,
									dormant_node_info * out_node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

									
status_t 
BMediaRoster::GetReadFileFormatsFor(const dormant_node_info & in_node,
									media_file_format * out_read_formats,
									int32 in_read_count,
									int32 * out_read_count)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

									
status_t 
BMediaRoster::GetWriteFileFormatsFor(const dormant_node_info & in_node,
									 media_file_format * out_write_formats,
									 int32 in_write_count,
									 int32 * out_write_count)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetFormatFor(const media_output & output,
						   media_format * io_format,
						   uint32 flags)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

						   
status_t 
BMediaRoster::GetFormatFor(const media_input & input,
						   media_format * io_format,
						   uint32 flags)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetFormatFor(const media_node & node,
						   media_format * io_format,
						   float quality)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

						   
ssize_t 
BMediaRoster::GetNodeAttributesFor(const media_node & node,
								   media_node_attribute * outArray,
								   size_t inMaxCount)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

								   
media_node_id 
BMediaRoster::NodeIDFor(port_id source_or_destination_port)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetInstancesFor(media_addon_id addon,
							  int32 flavor,
							  media_node_id * out_id,
							  int32 * io_count)
{
	UNIMPLEMENTED();
	return B_ERROR;
}



status_t 
BMediaRoster::SetRealtimeFlags(uint32 in_enabled)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


status_t 
BMediaRoster::GetRealtimeFlags(uint32 * out_enabled)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


ssize_t 
BMediaRoster::AudioBufferSizeFor(int32 channel_count,
								 uint32 sample_format,
								 float frame_rate,
								 bus_type bus_kind)
{
	UNIMPLEMENTED();
	return 0;
}

								 
/* Use MediaFlags to inquire about specific features of the Media Kit. */
/* Returns < 0 for "not present", positive size for output data size. */
/* 0 means that the capability is present, but no data about it. */
/* static */ ssize_t 
BMediaRoster::MediaFlags(media_flags cap,
						 void * buf,
						 size_t maxSize)
{
	UNIMPLEMENTED();
	return 0;
}

						 

/* BLooper overrides */
/* virtual */ void 
BMediaRoster::MessageReceived(BMessage * message)
{
	UNIMPLEMENTED();
}

/* virtual */ bool 
BMediaRoster::QuitRequested()
{
	UNIMPLEMENTED();
	return true;
}

/* virtual */ BHandler * 
BMediaRoster::ResolveSpecifier(BMessage *msg,
				 int32 index,
				 BMessage *specifier,
				 int32 form,
				 const char *property)
{
	UNIMPLEMENTED();
	return 0;
}				 


/* virtual */ status_t 
BMediaRoster::GetSupportedSuites(BMessage *data)
{
	UNIMPLEMENTED();
	return B_ERROR;
}


BMediaRoster::~BMediaRoster()
{
	CALLED();
	BMessage msg(MEDIA_SERVER_UNREGISTER_APP);
	BMessage reply;
	msg.AddInt32("team",team);
	ServerMessenger->SendMessage(&msg,&reply);
}


/*************************************************************
 * private BMediaRoster
 *************************************************************/

// deprecated call
status_t 
BMediaRoster::SetOutputBuffersFor(const media_source & output,
								  BBufferGroup * group,
								  bool will_reclaim )
{
	UNIMPLEMENTED();
	return B_ERROR;
}
	

/* FBC stuffing (Mmmh, Stuffing!) */
status_t BMediaRoster::_Reserved_MediaRoster_0(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_1(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_2(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_3(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_4(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_5(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_6(void *) { return B_ERROR; }
status_t BMediaRoster::_Reserved_MediaRoster_7(void *) { return B_ERROR; }


BMediaRoster::BMediaRoster() : 
	BLooper("BMediaRoster looper",B_NORMAL_PRIORITY,B_LOOPER_PORT_DEFAULT_CAPACITY)
{
	CALLED();
	BMessage msg(MEDIA_SERVER_REGISTER_APP);
	BMessage reply;
	msg.AddInt32("team",team);
	ServerMessenger->SendMessage(&msg,&reply);
}

/* static */ status_t
BMediaRoster::ParseCommand(BMessage & reply)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

		

status_t 
BMediaRoster::GetDefaultInfo(media_node_id for_default,
							 BMessage & out_config)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

		
							 
status_t 
BMediaRoster::SetRunningDefault(media_node_id for_default,
								const media_node & node)
{
	UNIMPLEMENTED();
	return B_ERROR;
}

		
void 
BMediaRoster::RegisterNode(media_node_id, BMediaNode*)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * static BMediaRoster variables
 *************************************************************/

bool BMediaRoster::_isMediaServer;
port_id BMediaRoster::_mReplyPort;
int32 BMediaRoster::_mReplyPortRes;
int32 BMediaRoster::_mReplyPortUnavailCount;
BMediaRoster * BMediaRoster::_sDefault = NULL;

