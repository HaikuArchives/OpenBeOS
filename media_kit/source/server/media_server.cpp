#include <Application.h>
#include <stdio.h>
#include <Messenger.h>
#include <MediaDefs.h>
#include <MediaFormats.h>
#include <Autolock.h>
#include "ServerInterface.h"

/*
 *
 * An implementation of a new media_server for the OpenBeOS MediaKit
 * Started by Marcus Overhagen <marcus@overhagen.de> on 2001-10-25
 * 
 * Communication with the OpenBeOS libmedia.so is done using BMessages 
 * sent to the server application, handled in LibInterface_XXX()
 * functions. A simple BMessage reply is beeing send back.
 *
 *
 * function names and class structure is loosely
 * based on information acquired using:
 * nm --demangle /boot/beos/system/servers/media_server | grep Server | sort 
 * nm --demangle /boot/beos/system/servers/media_server | grep Manager | sort
 *
 */


#define REPLY_TIMEOUT ((bigtime_t)100000)

class CAppManager
{
public:
	CAppManager();
	~CAppManager();
	bool HasTeam(team_id);
	status_t RegisterTeam(team_id, BMessenger);
	status_t UnregisterTeam(team_id);
	void BroadcastMessage(BMessage *msg, bigtime_t timeout);
	void HandleBroadcastError(BMessage *, BMessenger &, team_id team, bigtime_t timeout);
	status_t LoadState();
	status_t SaveState();
private:
	struct ListItem {
		team_id team;
		BMessenger messenger;
	};
	BList 	mList;
	BLocker	mLocker;
};

class CBufferManager;

class CNodeManager
{
public:
	CNodeManager();
	~CNodeManager();
	status_t RegisterNode(BMessenger, media_node &, char const *, long *, char const *, long, char const *, long, media_type, media_type);
	status_t UnregisterNode(long);
	status_t GetNodes(BMessage &, char const *);
	status_t GetLiveNode(BMessage &, char const *, long);
	status_t GetLiveNodes(BMessage &, char const *, media_format const *, media_format const *, char const *, unsigned long);
	status_t FindNode(long, media_node &);
	status_t FindSaveInfo(long, char const **, long *, long *, char const **);
	status_t FindDormantNodeFor(long, dormant_node_info *);
	status_t FindNodesFor(long, long, BMessage &, char const *);
	status_t FindNodesForPort(long, BMessage &, char const *);
	status_t UnregisterTeamNodes(long, BMessage &, char const *, long *, CBufferManager *);
	status_t IncrementGlobalRefCount(long);
	status_t DumpGlobalReferences(BMessage &, char const *);
	status_t DecrementGlobalRefCount(long, BMessage *);
	status_t BroadcastMessage(long, void *, long, long long);
	status_t LoadState();
	status_t SaveState();
};


class CServerApp : BApplication
{
public:
	CServerApp();
	~CServerApp();


	void LibInterface_GetNodeID(BMessage *);
	void LibInterface_FindRunningInstances(BMessage *);
	void LibInterface_BufferGroupReg(BMessage *);
	void LibInterface_GetLatentInfo(BMessage *);
	void LibInterface_GetDormantFileFormats(BMessage *);
	void LibInterface_GetDormantFlavor(BMessage *);
	void LibInterface_BroadcastMessage(BMessage *);
	void LibInterface_ReleaseNodeReference(BMessage *);
	void LibInterface_SetRealtimeFlags(BMessage *);
	void LibInterface_GetRealtimeFlags(BMessage *);
	void LibInterface_InstantiatePersistentNode(BMessage *);
	void LibInterface_SniffFile(BMessage *);
	void LibInterface_QueryLatents(BMessage *);
	void LibInterface_RegisterApp(BMessage *);
	void LibInterface_UnregisterApp(BMessage *);
	void LibInterface_RegisterNode(BMessage *);
	void LibInterface_UnregisterNode(BMessage *);
	void LibInterface_RegisterBuffer(BMessage *);
	void LibInterface_SetDefault(BMessage *);
	void LibInterface_AcquireNodeReference(BMessage *);
	void LibInterface_RequestNotifications(BMessage *);
	void LibInterface_CancelNotifications(BMessage *);
	void LibInterface_SetOutputBuffers(BMessage *);
	void LibInterface_ReclaimOutputBuffers(BMessage *);
	void LibInterface_OrphanReclaimableBuffers(BMessage *);
	void LibInterface_SetTimeSource(BMessage *);
	void LibInterface_QueryNodes(BMessage *);
	void LibInterface_GetDormantNode(BMessage *);
	void LibInterface_FormatChanged(BMessage *);
	void LibInterface_GetDefaultInfo(BMessage *);
	void LibInterface_GetRunningDefault(BMessage *);
	void LibInterface_SetRunningDefault(BMessage *);
	void LibInterface_TypeItemOp(BMessage *);
	void LibInterface_FormatOp(BMessage *);

	void LibInterface_SetVolume(BMessage *);
	void LibInterface_GetVolume(BMessage *);

/* functionality not yet implemented
00014a00 T _ServerApp::_ServerApp(void)
00014e1c T _ServerApp::~_ServerApp(void)
00014ff4 T _ServerApp::MessageReceived(BMessage *);
00015840 T _ServerApp::QuitRequested(void)
00015b50 T _ServerApp::_DoNotify(notify_data *)
00015d18 T _ServerApp::_UnregisterApp(long, bool)
00018e90 T _ServerApp::AddOnHost(void)
00019530 T _ServerApp::AboutRequested(void)
00019d04 T _ServerApp::AddPurgableBufferGroup(long, long, long, void *)
00019db8 T _ServerApp::CancelPurgableBufferGroupCleanup(long)
00019e50 T _ServerApp::DirtyWork(void)
0001a4bc T _ServerApp::ArgvReceived(long, char **)
0001a508 T _ServerApp::CleanupPurgedBufferGroup(_ServerApp::purgable_buffer_group const &, bool)
0001a5dc T _ServerApp::DirtyWorkLaunch(void *)
0001a634 T _ServerApp::SetQuitMode(bool)
0001a648 T _ServerApp::IsQuitMode(void) const
0001a658 T _ServerApp::BroadcastCurrentStateTo(BMessenger &)
0001adcc T _ServerApp::ReadyToRun(void)
*/

private:
	CAppManager *mAppManager;
	CNodeManager *mNodeManager;
	BLocker *mLocker;
	
	float mVolumeLeft;
	float mVolumeRight;

	void MessageReceived(BMessage *msg);
	typedef BApplication inherited;
};

CServerApp::CServerApp()
 	: BApplication(NEW_MEDIA_SERVER_SIGNATURE),
	mAppManager(new CAppManager),
	mNodeManager(new CNodeManager),
	mLocker(new BLocker),
	mVolumeLeft(0.0),
	mVolumeRight(0.0)
{
	//load volume settings from config file
	//mVolumeLeft = ???;
	//mVolumeRight = ???;
}

CServerApp::~CServerApp()
{
	delete mAppManager;
	delete mNodeManager;
	delete mLocker;
}


void CServerApp::LibInterface_GetNodeID(BMessage *msg)
{
}


void CServerApp::LibInterface_FindRunningInstances(BMessage *msg)
{
}


void CServerApp::LibInterface_BufferGroupReg(BMessage *msg)
{
}


void CServerApp::LibInterface_GetLatentInfo(BMessage *msg)
{
}


void CServerApp::LibInterface_GetDormantFileFormats(BMessage *msg)
{
}


void CServerApp::LibInterface_GetDormantFlavor(BMessage *msg)
{
}


void CServerApp::LibInterface_BroadcastMessage(BMessage *msg)
{
}


void CServerApp::LibInterface_ReleaseNodeReference(BMessage *msg)
{
}


void CServerApp::LibInterface_SetRealtimeFlags(BMessage *msg)
{
}


void CServerApp::LibInterface_GetRealtimeFlags(BMessage *msg)
{
}


void CServerApp::LibInterface_InstantiatePersistentNode(BMessage *msg)
{
}


void CServerApp::LibInterface_SniffFile(BMessage *msg)
{
}


void CServerApp::LibInterface_QueryLatents(BMessage *msg)
{
}


void CServerApp::LibInterface_RegisterApp(BMessage *msg)
{
	team_id team;
	msg->FindInt32("team", &team);
	mAppManager->RegisterTeam(team, msg->ReturnAddress());

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::LibInterface_UnregisterApp(BMessage *msg)
{
	team_id team;
	msg->FindInt32("team", &team);
	mAppManager->UnregisterTeam(team);

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::LibInterface_RegisterNode(BMessage *msg)
{
}


void CServerApp::LibInterface_UnregisterNode(BMessage *msg)
{
}


void CServerApp::LibInterface_RegisterBuffer(BMessage *msg)
{
}


void CServerApp::LibInterface_SetDefault(BMessage *msg)
{
}


void CServerApp::LibInterface_AcquireNodeReference(BMessage *msg)
{
}


void CServerApp::LibInterface_RequestNotifications(BMessage *msg)
{
}


void CServerApp::LibInterface_CancelNotifications(BMessage *msg)
{
}


void CServerApp::LibInterface_SetOutputBuffers(BMessage *msg)
{
}


void CServerApp::LibInterface_ReclaimOutputBuffers(BMessage *msg)
{
}


void CServerApp::LibInterface_OrphanReclaimableBuffers(BMessage *msg)
{
}


void CServerApp::LibInterface_SetTimeSource(BMessage *msg)
{
}


void CServerApp::LibInterface_QueryNodes(BMessage *msg)
{
}


void CServerApp::LibInterface_GetDormantNode(BMessage *msg)
{
}


void CServerApp::LibInterface_FormatChanged(BMessage *msg)
{
}


void CServerApp::LibInterface_GetDefaultInfo(BMessage *msg)
{
}


void CServerApp::LibInterface_GetRunningDefault(BMessage *msg)
{
}


void CServerApp::LibInterface_SetRunningDefault(BMessage *msg)
{
}


void CServerApp::LibInterface_TypeItemOp(BMessage *msg)
{
}


void CServerApp::LibInterface_FormatOp(BMessage *msg)
{
}

void CServerApp::LibInterface_SetVolume(BMessage *msg)
{
	float left;
	float right;
	msg->FindFloat("left", &left);
	msg->FindFloat("right", &right);

	mLocker->Lock();
	mVolumeLeft = left;
	mVolumeRight = right;
	mLocker->Unlock();

	//save volume settings to config file
	// ??? = left;
	// ??? = right;

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}

void CServerApp::LibInterface_GetVolume(BMessage *msg)
{
	BMessage reply(B_OK);

	mLocker->Lock();
	reply.AddFloat("left", mVolumeLeft);
	reply.AddFloat("right", mVolumeRight);
	mLocker->Unlock();

	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MEDIA_SERVER_GET_NODE_ID: LibInterface_GetNodeID(msg); break;
		case MEDIA_SERVER_FIND_RUNNING_INSTANCES: LibInterface_FindRunningInstances(msg); break;
		case MEDIA_SERVER_BUFFER_GROUP_REG: LibInterface_BufferGroupReg(msg); break;
		case MEDIA_SERVER_GET_LATENT_INFO: LibInterface_GetLatentInfo(msg); break;
		case MEDIA_SERVER_GET_DORMANT_FILE_FORMATS: LibInterface_GetDormantFileFormats(msg); break;
		case MEDIA_SERVER_GET_GETDORMANTFLAVOR: LibInterface_GetDormantFlavor(msg); break;
		case MEDIA_SERVER_BROADCAST_MESSAGE: LibInterface_BroadcastMessage(msg); break;
		case MEDIA_SERVER_RELEASE_NODE_REFERENCE: LibInterface_ReleaseNodeReference(msg); break;
		case MEDIA_SERVER_SET_REALTIME_FLAGS: LibInterface_SetRealtimeFlags(msg); break;
		case MEDIA_SERVER_GET_REALTIME_FLAGS: LibInterface_GetRealtimeFlags(msg); break;
		case MEDIA_SERVER_INSTANTIATE_PERSISTENT_NODE: LibInterface_InstantiatePersistentNode(msg); break;
		case MEDIA_SERVER_SNIFF_FILE: LibInterface_SniffFile(msg); break;
		case MEDIA_SERVER_QUERY_LATENTS: LibInterface_QueryLatents(msg); break;
		case MEDIA_SERVER_REGISTER_APP: LibInterface_RegisterApp(msg); break;
		case MEDIA_SERVER_UNREGISTER_APP: LibInterface_UnregisterApp(msg); break;
		case MEDIA_SERVER_REGISTER_NODE: LibInterface_RegisterNode(msg); break;
		case MEDIA_SERVER_UNREGISTER_NODE: LibInterface_UnregisterNode(msg); break;
		case MEDIA_SERVER_REGISTER_BUFFER: LibInterface_RegisterBuffer(msg); break;
		case MEDIA_SERVER_SET_DEFAULT: LibInterface_SetDefault(msg); break;
		case MEDIA_SERVER_ACQUIRE_NODE_REFERENCE: LibInterface_AcquireNodeReference(msg); break;
		case MEDIA_SERVER_REQUEST_NOTIFICATIONS: LibInterface_RequestNotifications(msg); break;
		case MEDIA_SERVER_CANCEL_NOTIFICATIONS: LibInterface_CancelNotifications(msg); break;
		case MEDIA_SERVER_SET_OUTPUT_BUFFERS: LibInterface_SetOutputBuffers(msg); break;
		case MEDIA_SERVER_RECLAIM_OUTPUT_BUFFERS: LibInterface_ReclaimOutputBuffers(msg); break;
		case MEDIA_SERVER_ORPHAN_RECLAIMABLE_BUFFERS: LibInterface_OrphanReclaimableBuffers(msg); break;
		case MEDIA_SERVER_SET_TIME_SOURCE: LibInterface_SetTimeSource(msg); break;
		case MEDIA_SERVER_QUERY_NODES: LibInterface_QueryNodes(msg); break;
		case MEDIA_SERVER_GET_DORMANT_NODE: LibInterface_GetDormantNode(msg); break;
		case MEDIA_SERVER_FORMAT_CHANGED: LibInterface_FormatChanged(msg); break;
		case MEDIA_SERVER_GET_DEFAULT_INFO: LibInterface_GetDefaultInfo(msg); break;
		case MEDIA_SERVER_GET_RUNNING_DEFAULT: LibInterface_GetRunningDefault(msg); break;
		case MEDIA_SERVER_SET_RUNNING_DEFAULT: LibInterface_SetRunningDefault(msg); break;
		case MEDIA_SERVER_TYPE_ITEM_OP: LibInterface_TypeItemOp(msg); break;
		case MEDIA_SERVER_FORMAT_OP: LibInterface_FormatOp(msg); break;
		case MEDIA_SERVER_SET_VOLUME: LibInterface_SetVolume(msg); break;
		case MEDIA_SERVER_GET_VOLUME: LibInterface_GetVolume(msg); break;
		default:
			printf("\nnew media server: unknown message received\n");
			msg->PrintToStream();
	}
}

int main()
{
	new CServerApp;
	be_app->Run();
	delete be_app;
	return 0;
}

CAppManager::CAppManager()
{
}

CAppManager::~CAppManager()
{
}

bool CAppManager::HasTeam(team_id team)
{
	BAutolock lock(mLocker);
	ListItem *item;
	for (int32 i = 0; (item = (ListItem *)mList.ItemAt(i)) != NULL; i++)
		if (item->team == team)
			return true;
	return false;
}

status_t CAppManager::RegisterTeam(team_id team, BMessenger messenger)
{
	printf("CAppManager::RegisterTeam %d\n",(int) team);

	BAutolock lock(mLocker);
	ListItem *item;

	if (HasTeam(team))
		return B_ERROR;

	item = new ListItem;
	item->team = team;
	item->messenger = messenger;

	return mList.AddItem(item) ? B_OK : B_ERROR;
}

status_t CAppManager::UnregisterTeam(team_id team)
{
	printf("CAppManager::UnregisterTeam %d\n",(int) team);

	BAutolock lock(mLocker);
	ListItem *item;
	for (int32 i = 0; (item = (ListItem *)mList.ItemAt(i)) != NULL; i++)
		if (item->team == team) {
			if (mList.RemoveItem(item)) {
				delete item;
				return B_OK;
			} else {
				break;
			}
		}
	return B_ERROR;
}

void CAppManager::BroadcastMessage(BMessage *msg, bigtime_t timeout)
{
	BAutolock lock(mLocker);
	ListItem *item;
	for (int32 i = 0; (item = (ListItem *)mList.ItemAt(i)) != NULL; i++)
		if (B_OK != item->messenger.SendMessage(msg,(BHandler *)NULL,timeout))
			HandleBroadcastError(msg, item->messenger, item->team, timeout);
}

void CAppManager::HandleBroadcastError(BMessage *msg, BMessenger &, team_id team, bigtime_t timeout)
{
	BAutolock lock(mLocker);
	printf("error broadcasting team %d with message after %.3f seconds\n",int(team),timeout / 1000000.0);
	msg->PrintToStream();	
}

status_t CAppManager::LoadState()
{
	return B_ERROR;
}

status_t CAppManager::SaveState()
{
	return B_ERROR;
}

CNodeManager::CNodeManager()
{
}

CNodeManager::~CNodeManager()
{
}

/*
0001e260 T MBufferManager::MBufferManager(void)
0001e47c T MBufferManager::~MBufferManager(void)
0001e540 T MBufferManager::PrintToStream(void)
0001e6fc T MBufferManager::RecycleBuffersWithOwner(long, long)
0001ea00 T MBufferManager::RegisterBuffer(long, buffer_clone_info const *, long *, media_source const &)
0001f090 T MBufferManager::AcquireBuffer(long, long, media_source)
0001f28c T MBufferManager::ReleaseBuffer(long, long, bool, BMessage *, char const *)
0001fd0c T MBufferManager::PurgeTeamBufferGroups(long)
0001fdf0 T MBufferManager::RegisterBufferGroup(BMessage *, char const *, long)
0002007c T MBufferManager::_RemoveGroupFromClaimants(long, long, void *)
00020158 T MBufferManager::UnregisterBufferGroup(BMessage *, char const *, long)
0002028c T MBufferManager::_UnregisterBufferGroup(long, long)
000206f4 T MBufferManager::AddBuffersTo(BMessage *, char const *)
00020cd0 T MBufferManager::ReclaimBuffers(long const *, long, long, long)
00020f7c T MBufferManager::CleanupPurgedBufferGroup(long, long, long, void *, bool, BMessage &)
00021080 T MBufferManager::LoadState(void)
0002108c T MBufferManager::SaveState(void)

000210a0 T MDefaultManager::MDefaultManager(void)
0002123c T MDefaultManager::~MDefaultManager(void)
000212f4 T MDefaultManager::SaveState(void)
0002172c T MDefaultManager::LoadState(void)
00021de0 T MDefaultManager::SetDefault(long, BMessage &)
00022058 T MDefaultManager::SetRunningDefault(long, media_node const &)
000221a8 T MDefaultManager::RemoveRunningDefault(media_node const &)
000226ec T MDefaultManager::SetRealtimeFlags(unsigned long)
00022720 T MDefaultManager::GetRealtimeFlags(void)
00022730 T MDefaultManager::GetRunningDefault(long, media_node &)
000227d0 T MDefaultManager::RemoveDefault(long)
00022830 T MDefaultManager::GetDefault(long, BMessage &)
00022890 T MNotifierManager::MNotifierManager(void)
00022a5c T MNotifierManager::~MNotifierManager(void)
00022b20 T MNotifierManager::RegisterNotifier(long, BMessenger, media_node const *)
00022f50 T MNotifierManager::UnregisterNotifier(long, BMessenger, media_node const *)
00023f00 T MNotifierManager::BroadcastMessage(BMessage *, long long)
0002426c T MNotifierManager::regen_node_list(media_node const &)
000249b4 T MNotifierManager::get_node_messenger(media_node const &, get_messenger_a *)
00024a90 T MNotifierManager::HandleBroadcastError(BMessage *, BMessenger &, long, long long)
00024b34 T MNotifierManager::LoadState(void)
00024b40 T MNotifierManager::SaveState(void)
00024c5c T MMediaFilesManager::MMediaFilesManager(void)
00024dec T MMediaFilesManager::~MMediaFilesManager(void)
00024ea4 T MMediaFilesManager::SaveState(void)
00025b70 T MMediaFilesManager::LoadState(void)
00026668 T MMediaFilesManager::create_default_settings(void)
00027130 T MMediaFilesManager::GetTypes(BMessage &, BMessage &)
000271f8 T MMediaFilesManager::GetItems(BMessage &, BMessage &)
000274f0 T MMediaFilesManager::SetItem(BMessage &, BMessage &)
00027bc0 T MMediaFilesManager::ClearItem(BMessage &, BMessage &)
000288f4 T MMediaFilesManager::RemoveItem(BMessage &, BMessage &)
00028f0c T MMediaFilesManager::AddType(BMessage &, BMessage &) 
*/

