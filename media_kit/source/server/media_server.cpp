#include <Application.h>
#include <stdio.h>
#include <Messenger.h>
#include <MediaDefs.h>
#include <MediaFormats.h>
#include <Autolock.h>
#include "ServerInterface.h"
#include "BufferManager.h"
#include "NodeManager.h"
#include "AppManager.h"
#include "debug.h"

/*
 *
 * An implementation of a new media_server for the OpenBeOS MediaKit
 * Started by Marcus Overhagen <marcus@overhagen.de> on 2001-10-25
 * 
 * Communication with the OpenBeOS libmedia.so is done using BMessages 
 * sent to the server application, handled in XXX()
 * functions. A simple BMessage reply is beeing send back.
 *
 *
 * function names and class structure is loosely
 * based on information acquired using:
 * nm --demangle /boot/beos/system/servers/media_server | grep Server | sort 
 * nm --demangle /boot/beos/system/servers/media_server | grep Manager | sort
 *
 */


#define REPLY_TIMEOUT ((bigtime_t)500000)

class CServerApp : BApplication
{
public:
	CServerApp();
	~CServerApp();

	void GetSharedBufferArea(BMessage *msg);
	void RegisterBuffer(BMessage *msg);
	void UnregisterBuffer(BMessage *msg);

	void GetNodeID(BMessage *);
	void FindRunningInstances(BMessage *);
	void BufferGroupReg(BMessage *);
	void GetLatentInfo(BMessage *);
	void GetDormantFileFormats(BMessage *);
	void GetDormantFlavor(BMessage *);
	void BroadcastMessage(BMessage *);
	void ReleaseNodeReference(BMessage *);
	void SetRealtimeFlags(BMessage *);
	void GetRealtimeFlags(BMessage *);
	void InstantiatePersistentNode(BMessage *);
	void SniffFile(BMessage *);
	void QueryLatents(BMessage *);
	void RegisterApp(BMessage *);
	void UnregisterApp(BMessage *);
	void RegisterNode(BMessage *);
	void UnregisterNode(BMessage *);
	void SetDefault(BMessage *);
	void AcquireNodeReference(BMessage *);
	void RequestNotifications(BMessage *);
	void CancelNotifications(BMessage *);
	void SetOutputBuffers(BMessage *);
	void ReclaimOutputBuffers(BMessage *);
	void OrphanReclaimableBuffers(BMessage *);
	void SetTimeSource(BMessage *);
	void QueryNodes(BMessage *);
	void GetDormantNode(BMessage *);
	void FormatChanged(BMessage *);
	void GetDefaultInfo(BMessage *);
	void GetRunningDefault(BMessage *);
	void SetRunningDefault(BMessage *);
	void TypeItemOp(BMessage *);
	void FormatOp(BMessage *);

	void SetVolume(BMessage *);
	void GetVolume(BMessage *);

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
	BufferManager *fBufferManager;
	AppManager *fAppManager;
	NodeManager *fNodeManager;
	BLocker *fLocker;
	
	float fVolumeLeft;
	float fVolumeRight;

	void MessageReceived(BMessage *msg);
	typedef BApplication inherited;
};

CServerApp::CServerApp()
 	: BApplication(NEW_MEDIA_SERVER_SIGNATURE),
 	fBufferManager(new BufferManager),
	fAppManager(new AppManager),
	fNodeManager(new NodeManager),
	fLocker(new BLocker("server locker")),
	fVolumeLeft(0.0),
	fVolumeRight(0.0)
{
	//load volume settings from config file
	//mVolumeLeft = ???;
	//mVolumeRight = ???;
}

CServerApp::~CServerApp()
{
	delete fBufferManager;
	delete fAppManager;
	delete fNodeManager;
	delete fLocker;
}

void
CServerApp::GetSharedBufferArea(BMessage *msg)
{
	BMessage reply(B_OK);
	reply.AddInt32("shared buffer area",fBufferManager->SharedBufferListID());
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}

void
CServerApp::RegisterBuffer(BMessage *msg)
{
	team_id teamid;
	media_buffer_id bufferid;
	size_t size;
	int32 flags;
	size_t offset;
	area_id area;
	status_t status;
	
	//msg->PrintToStream();
	
	teamid = 	msg->FindInt32("team");
	area = 		msg->FindInt32("area");
	offset =	msg->FindInt32("offset");
	size = 		msg->FindInt32("size");
	flags = 	msg->FindInt32("flags");
	bufferid = 	msg->FindInt32("buffer");

	//TRACE("ServerApp::RegisterBuffer team = 0x%08x, areaid = 0x%08x, offset = 0x%08x, size = 0x%08x, flags = 0x%08x, buffer = 0x%08x\n",(int)teamid,(int)area,(int)offset,(int)size,(int)flags,(int)bufferid);

	if (bufferid == 0)
		status = fBufferManager->RegisterBuffer(teamid, size, flags, offset, area, &bufferid);
	else
		status = fBufferManager->RegisterBuffer(teamid, bufferid, &size, &flags, &offset, &area);

	BMessage reply(status);
	reply.AddInt32("buffer",bufferid);
	reply.AddInt32("size",size);
	reply.AddInt32("flags",flags);
	reply.AddInt32("offset",offset);
	reply.AddInt32("area",area);

	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}

void
CServerApp::UnregisterBuffer(BMessage *msg)
{
	team_id teamid;
	media_buffer_id bufferid;
	status_t status;

	teamid = msg->FindInt32("team");
	bufferid = msg->FindInt32("buffer");
	
	status = fBufferManager->UnregisterBuffer(teamid, bufferid);

	BMessage reply(status);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::GetNodeID(BMessage *msg)
{
}


void CServerApp::FindRunningInstances(BMessage *msg)
{
}


void CServerApp::BufferGroupReg(BMessage *msg)
{
}


void CServerApp::GetLatentInfo(BMessage *msg)
{
}


void CServerApp::GetDormantFileFormats(BMessage *msg)
{
}


void CServerApp::GetDormantFlavor(BMessage *msg)
{
}


void CServerApp::BroadcastMessage(BMessage *msg)
{
}


void CServerApp::ReleaseNodeReference(BMessage *msg)
{
}


void CServerApp::SetRealtimeFlags(BMessage *msg)
{
}


void CServerApp::GetRealtimeFlags(BMessage *msg)
{
}


void CServerApp::InstantiatePersistentNode(BMessage *msg)
{
}


void CServerApp::SniffFile(BMessage *msg)
{
}


void CServerApp::QueryLatents(BMessage *msg)
{
}


void CServerApp::RegisterApp(BMessage *msg)
{
	team_id team;
	msg->FindInt32("team", &team);
	fAppManager->RegisterTeam(team, msg->ReturnAddress());

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::UnregisterApp(BMessage *msg)
{
	team_id team;
	msg->FindInt32("team", &team);
	fAppManager->UnregisterTeam(team);

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::RegisterNode(BMessage *msg)
{
}


void CServerApp::UnregisterNode(BMessage *msg)
{
}


void CServerApp::SetDefault(BMessage *msg)
{
}


void CServerApp::AcquireNodeReference(BMessage *msg)
{
}


void CServerApp::RequestNotifications(BMessage *msg)
{
}


void CServerApp::CancelNotifications(BMessage *msg)
{
}


void CServerApp::SetOutputBuffers(BMessage *msg)
{
}


void CServerApp::ReclaimOutputBuffers(BMessage *msg)
{
}


void CServerApp::OrphanReclaimableBuffers(BMessage *msg)
{
}


void CServerApp::SetTimeSource(BMessage *msg)
{
}


void CServerApp::QueryNodes(BMessage *msg)
{
}


void CServerApp::GetDormantNode(BMessage *msg)
{
}


void CServerApp::FormatChanged(BMessage *msg)
{
}


void CServerApp::GetDefaultInfo(BMessage *msg)
{
}


void CServerApp::GetRunningDefault(BMessage *msg)
{
}


void CServerApp::SetRunningDefault(BMessage *msg)
{
}


void CServerApp::TypeItemOp(BMessage *msg)
{
}


void CServerApp::FormatOp(BMessage *msg)
{
}

void CServerApp::SetVolume(BMessage *msg)
{
	float left;
	float right;
	msg->FindFloat("left", &left);
	msg->FindFloat("right", &right);

	fLocker->Lock();
	fVolumeLeft = left;
	fVolumeRight = right;
	fLocker->Unlock();

	//save volume settings to config file
	// ??? = left;
	// ??? = right;

	BMessage reply(B_OK);
	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}

void CServerApp::GetVolume(BMessage *msg)
{
	BMessage reply(B_OK);

	fLocker->Lock();
	reply.AddFloat("left", fVolumeLeft);
	reply.AddFloat("right", fVolumeRight);
	fLocker->Unlock();

	msg->SendReply(&reply,(BHandler*)NULL,REPLY_TIMEOUT);
}


void CServerApp::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MEDIA_SERVER_GET_SHARED_BUFFER_AREA: GetSharedBufferArea(msg); break;
		case MEDIA_SERVER_REGISTER_BUFFER: RegisterBuffer(msg); break;
		case MEDIA_SERVER_UNREGISTER_BUFFER: UnregisterBuffer(msg); break;
	
	
		case MEDIA_SERVER_GET_NODE_ID: GetNodeID(msg); break;
		case MEDIA_SERVER_FIND_RUNNING_INSTANCES: FindRunningInstances(msg); break;
		case MEDIA_SERVER_BUFFER_GROUP_REG: BufferGroupReg(msg); break;
		case MEDIA_SERVER_GET_LATENT_INFO: GetLatentInfo(msg); break;
		case MEDIA_SERVER_GET_DORMANT_FILE_FORMATS: GetDormantFileFormats(msg); break;
		case MEDIA_SERVER_GET_GETDORMANTFLAVOR: GetDormantFlavor(msg); break;
		case MEDIA_SERVER_BROADCAST_MESSAGE: BroadcastMessage(msg); break;
		case MEDIA_SERVER_RELEASE_NODE_REFERENCE: ReleaseNodeReference(msg); break;
		case MEDIA_SERVER_SET_REALTIME_FLAGS: SetRealtimeFlags(msg); break;
		case MEDIA_SERVER_GET_REALTIME_FLAGS: GetRealtimeFlags(msg); break;
		case MEDIA_SERVER_INSTANTIATE_PERSISTENT_NODE: InstantiatePersistentNode(msg); break;
		case MEDIA_SERVER_SNIFF_FILE: SniffFile(msg); break;
		case MEDIA_SERVER_QUERY_LATENTS: QueryLatents(msg); break;
		case MEDIA_SERVER_REGISTER_APP: RegisterApp(msg); break;
		case MEDIA_SERVER_UNREGISTER_APP: UnregisterApp(msg); break;
		case MEDIA_SERVER_REGISTER_NODE: RegisterNode(msg); break;
		case MEDIA_SERVER_UNREGISTER_NODE: UnregisterNode(msg); break;
		case MEDIA_SERVER_SET_DEFAULT: SetDefault(msg); break;
		case MEDIA_SERVER_ACQUIRE_NODE_REFERENCE: AcquireNodeReference(msg); break;
		case MEDIA_SERVER_REQUEST_NOTIFICATIONS: RequestNotifications(msg); break;
		case MEDIA_SERVER_CANCEL_NOTIFICATIONS: CancelNotifications(msg); break;
		case MEDIA_SERVER_SET_OUTPUT_BUFFERS: SetOutputBuffers(msg); break;
		case MEDIA_SERVER_RECLAIM_OUTPUT_BUFFERS: ReclaimOutputBuffers(msg); break;
		case MEDIA_SERVER_ORPHAN_RECLAIMABLE_BUFFERS: OrphanReclaimableBuffers(msg); break;
		case MEDIA_SERVER_SET_TIME_SOURCE: SetTimeSource(msg); break;
		case MEDIA_SERVER_QUERY_NODES: QueryNodes(msg); break;
		case MEDIA_SERVER_GET_DORMANT_NODE: GetDormantNode(msg); break;
		case MEDIA_SERVER_FORMAT_CHANGED: FormatChanged(msg); break;
		case MEDIA_SERVER_GET_DEFAULT_INFO: GetDefaultInfo(msg); break;
		case MEDIA_SERVER_GET_RUNNING_DEFAULT: GetRunningDefault(msg); break;
		case MEDIA_SERVER_SET_RUNNING_DEFAULT: SetRunningDefault(msg); break;
		case MEDIA_SERVER_TYPE_ITEM_OP: TypeItemOp(msg); break;
		case MEDIA_SERVER_FORMAT_OP: FormatOp(msg); break;
		case MEDIA_SERVER_SET_VOLUME: SetVolume(msg); break;
		case MEDIA_SERVER_GET_VOLUME: GetVolume(msg); break;
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

