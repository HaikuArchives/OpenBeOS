#ifndef _SERVER_INTERFACE_H_
#define _SERVER_INTERFACE_H_

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

// implementation contained in libmedia.so (MediaRoster.cpp)
namespace MediaKitPrivate {
	status_t QueryServer(BMessage &reply, const BMessage &query);
};

#endif
