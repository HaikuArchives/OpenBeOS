#ifndef _SERVER_INTERFACE_H_
#define _SERVER_INTERFACE_H_

#include <MediaDefs.h>
#include <MediaNode.h>

#define NEW_MEDIA_SERVER_SIGNATURE 	"application/x-vnd.OpenBeOS-media-server"

enum {
	// Application management
	MEDIA_SERVER_REGISTER_APP,
	MEDIA_SERVER_UNREGISTER_APP,

	// Buffer management
	MEDIA_SERVER_GET_SHARED_BUFFER_AREA,
	MEDIA_SERVER_REGISTER_BUFFER,
	MEDIA_SERVER_UNREGISTER_BUFFER,

	// Something else
	MEDIA_SERVER_SET_VOLUME,
	MEDIA_SERVER_GET_VOLUME,
	MEDIA_SERVER_GET_NODE_ID,
	MEDIA_SERVER_FIND_RUNNING_INSTANCES,
	MEDIA_SERVER_BUFFER_GROUP_REG,
	MEDIA_SERVER_GET_LATENT_INFO,
	MEDIA_SERVER_GET_DORMANT_FILE_FORMATS,
	MEDIA_SERVER_GET_GETDORMANTFLAVOR,
	MEDIA_SERVER_BROADCAST_MESSAGE,
	MEDIA_SERVER_RELEASE_NODE_REFERENCE,
	MEDIA_SERVER_SET_REALTIME_FLAGS,
	MEDIA_SERVER_GET_REALTIME_FLAGS,
	MEDIA_SERVER_INSTANTIATE_PERSISTENT_NODE,
	MEDIA_SERVER_SNIFF_FILE,
	MEDIA_SERVER_QUERY_LATENTS,
	MEDIA_SERVER_REGISTER_NODE,
	MEDIA_SERVER_UNREGISTER_NODE,
	MEDIA_SERVER_SET_DEFAULT,
	MEDIA_SERVER_ACQUIRE_NODE_REFERENCE,
	MEDIA_SERVER_REQUEST_NOTIFICATIONS,
	MEDIA_SERVER_CANCEL_NOTIFICATIONS,
	MEDIA_SERVER_SET_OUTPUT_BUFFERS,
	MEDIA_SERVER_RECLAIM_OUTPUT_BUFFERS,
	MEDIA_SERVER_ORPHAN_RECLAIMABLE_BUFFERS,
	MEDIA_SERVER_SET_TIME_SOURCE,
	MEDIA_SERVER_QUERY_NODES,
	MEDIA_SERVER_GET_DORMANT_NODE,
	MEDIA_SERVER_FORMAT_CHANGED,
	MEDIA_SERVER_GET_DEFAULT_INFO,
	MEDIA_SERVER_GET_RUNNING_DEFAULT,
	MEDIA_SERVER_SET_RUNNING_DEFAULT,
	MEDIA_SERVER_TYPE_ITEM_OP,
	MEDIA_SERVER_FORMAT_OP
};

enum {
	NODE_START,
	NODE_STOP,
	NODE_SEEK,
	NODE_SET_RUN_MODE,
	NODE_TIME_WARP,
	NODE_PREROLL,
	NODE_REGISTERED,
	NODE_SET_TIMESOURCE,
	CONSUMER_ACCEPT_FORMAT,
	CONSUMER_GET_NEXT_INPUT,
	CONSUMER_DISPOSE_INPUT_COOKIE,
	CONSUMER_BUFFER_RECEIVED,
	CONSUMER_PRODUCER_DATA_STATUS,
	CONSUMER_GET_LATENCY_FOR,
	CONSUMER_CONNECTED,
	CONSUMER_DISCONNECTED,
	CONSUMER_FORMAT_CHANGED,
	CONSUMER_SEEK_TAG_REQUESTED,
	PRODUCER_LATE_NOTICE,
	PRODUCER_OUTPUT_ENABLE,
	PRODUCER_LATENCY_CHANGE,
	PRODUCER_ADDITIONAL_BUFFER_REQUESTED,
	NODE_REQUEST_COMPLETED,
	END
};

struct xfer_producer_additional_buffer_requested
{
	media_source source;
	media_buffer_id prev_buffer;
	bigtime_t prev_time;
	bool has_seek_tag;
	media_seek_tag prev_tag;
};

struct xfer_producer_latency_change
{
	media_source source;
	media_destination destination;
	bigtime_t latency;
	uint32 flags;
};

struct xfer_node_request_completed
{
	media_request_info info;
};

struct xfer_producer_output_enable
{
	media_source source;
	media_destination destination;
	bool enabled;
	void *user_data;
	int32 change_tag;
};

struct xfer_producer_late_notice
{
	media_source source;
	bigtime_t how_much;
	bigtime_t performance_time;
};

struct xfer_node_start
{
	bigtime_t performance_time;	
};

struct xfer_node_stop
{
	bigtime_t performance_time;	
	bool immediate;
};

struct xfer_node_seek
{
	bigtime_t media_time;
	bigtime_t performance_time;
};

struct xfer_node_set_run_mode
{
	BMediaNode::run_mode mode;
};

struct xfer_node_time_warp
{
	bigtime_t at_real_time;
	bigtime_t to_performance_time;
};

struct xfer_node_registered
{
	media_node_id node_id;
};

struct xfer_node_set_timesource
{
	media_node_id timesource_id;
};

struct xfer_producer_accept_format
{
	media_destination dest;
	media_format format;
	port_id reply_port;
};

struct xfer_producer_accept_format_reply
{
	media_format format;
	status_t result;
};

struct xfer_producer_get_next_input
{
	int32 cookie;
	port_id reply_port;
};

struct xfer_producer_get_next_input_reply
{
	int32 cookie;
	media_input input;
	status_t result;
};

struct xfer_producer_dispose_input_cookie
{
	int32 cookie;
};

struct xfer_producer_buffer_received
{
	media_buffer_id buffer;
	media_header header;
};

struct xfer_producer_data_status
{
	media_destination for_whom;
	int32 status;
	bigtime_t at_performance_time;
};

struct xfer_producer_get_latency_for
{
	media_destination for_whom;
	port_id reply_port;
};

struct xfer_producer_get_latency_for_reply
{
	bigtime_t latency;
	media_node_id timesource;
	status_t result;
};
	
struct xfer_producer_connected
{
	media_source producer;
	media_destination where;
	media_format with_format;
	port_id reply_port;
};

struct xfer_producer_connected_reply
{
	media_input input;
	status_t result;
};

struct xfer_producer_disconnected
{
	media_source producer;
	media_destination where;
};

struct xfer_producer_format_changed
{
	media_source producer;
	media_destination consumer;
	int32 change_tag;
	media_format format;
	port_id reply_port;
};

struct xfer_producer_format_changed_reply
{
	status_t result;
};

struct xfer_producer_seek_tag_requested
{
	media_destination destination;
	bigtime_t target_time;
	uint32 flags;
	port_id reply_port;
};

struct xfer_producer_seek_tag_requested_reply
{
	media_seek_tag seek_tag;
	bigtime_t tagged_time;
	uint32 flags;
	status_t result;
};


// implementation contained in libmedia.so (MediaRoster.cpp)
namespace MediaKitPrivate 
{
	status_t QueryServer(BMessage *query, BMessage *reply);
};

#endif
