#define BUILDING_MEDIA_ADDON 1
#include <Application.h>
#include <Alert.h>
#include <MediaRoster.h>
#include <NodeMonitor.h>
#include <MediaAddOn.h>
#include <String.h>
#include <stdio.h>
#include <Entry.h>
#include <Path.h>
#include <Directory.h>
#include <Looper.h>
#include "TMap.h"
#include "../server/headers/ServerInterface.h"

media_addon_id nextid = 1;

class Application : BApplication
{
public:
	Application(const char *sig);
	~Application();
	void ReadyToRun();
	void MessageReceived(BMessage *msg);
	
	void WatchDir(BEntry *dir);
	void AddOnAdded(const char *path, ino_t file_node);
	void AddOnRemoved(ino_t file_node);
	void HandleMessage(int32 code, void *data, size_t size);
	static int32 controlthread(void *arg);

	Map<ino_t,media_addon_id> *filemap;
	Map<media_addon_id,BMediaAddon *> *addonmap;

	BMediaRoster *mediaroster;
	ino_t		DirNodeSystem;
	ino_t		DirNodeUser;
	port_id		control_port;
	thread_id	control_thread;
};

Application::Application(const char *sig) : 
	BApplication(sig)
{
	mediaroster = BMediaRoster::Roster();
	filemap = new Map<ino_t,media_addon_id>;
	addonmap = new Map<media_addon_id,BMediaAddon *>;
	control_port = create_port(64,"media_addon_server port");
	control_thread = spawn_thread(controlthread,"media_addon_server control",12,this);
	resume_thread(control_thread);
}

Application::~Application()
{
	delete_port(control_port);
	status_t err;
	wait_for_thread(control_thread,&err);

	// delete all BMediaAddon objects
	BMediaAddon *addon;
	for (int32 index = 0; addonmap->GetAt(index,&addon); index++)
		delete addon;
	
	delete addonmap;
	delete filemap;
}

void 
Application::HandleMessage(int32 code, void *data, size_t size)
{
	switch (code) {
		case ADDONSERVER_INSTANTIATE_DORMANT_NODE:
		{
			const xfer_addonserver_instantiate_dormant_node *msg = (const xfer_addonserver_instantiate_dormant_node *)data;
			xfer_addonserver_instantiate_dormant_node_reply reply;
			reply.result = mediaroster->InstantiateDormantNode(msg->info, &reply.node);
			write_port(msg->reply_port, 0, &reply, sizeof(reply));
			break;
		}
	}
}

int32
Application::controlthread(void *arg)
{
	char data[B_MEDIA_MESSAGE_SIZE];
	Application *app;
	ssize_t size;
	int32 code;
	
	app = (Application *)arg;
	while ((size = read_port_etc(app->control_port, &code, data, sizeof(data), 0, 0)) > 0)
		app->HandleMessage(code, data, size);

	return 0;
}

void 
Application::ReadyToRun()
{
	node_ref nref;

	BEntry e("/boot/beos/system/add-ons/media");
	e.GetNodeRef(&nref);
	DirNodeSystem = nref.node;
	WatchDir(&e);

	BEntry e2("/boot/home/config/add-ons/media");
	e2.GetNodeRef(&nref);
	DirNodeUser = nref.node;
	WatchDir(&e2);
}

void 
Application::AddOnAdded(const char *path, ino_t file_node)
{
	printf("loading BMediaAddOn from %s\n",path);

	BMediaAddOn *(*make_addon)(image_id you);
	BMediaAddOn *addon = 0;
	image_id image;
	status_t rv;
	
	image = load_add_on(path);
	if (image < B_OK) {
		printf("loading failed %lx %s\n", image, strerror(image));
		return;
	}
	
	rv = get_image_symbol(image, "make_media_addon", B_SYMBOL_TYPE_TEXT, (void**)&make_addon);
	if (rv < B_OK) {
		printf("loading failed, function not found %lx %s\n", rv, strerror(rv));
		return;
	}
	
	addon = make_addon(image);
	if (addon == 0) {
		printf("creating BMediaAddOn failed\n");
		goto finished;
	}
/*	
	const char *error;
	rv = addon->InitCheck(&error);
	if (rv != B_OK) {
		printf("BMediaAddOn::InitCheck() failed, %lx %s \"%s\"\n", rv, strerror(rv), error);
		goto finished;
	}
*/
	int flavorcount;
	flavorcount = addon->CountFlavors();
	printf("%d flavors:\n",flavorcount);
	for (int i = 0; i < flavorcount; i++) {
		const flavor_info *info;
		printf("flavor %d:\n",i);
		if (B_OK != addon->GetFlavorAt(i, &info)) {
			printf("failed!\n");
			continue;
		}
		printf("  internal_id = %ld\n",info->internal_id);
		printf("  name = %s\n",info->name);
		printf("  info = %s\n",info->info);
		printf("  flavor_flags = 0x%lx",info->flavor_flags);
		if (info->flavor_flags & B_FLAVOR_IS_GLOBAL) printf(" B_FLAVOR_IS_GLOBAL");
		if (info->flavor_flags & B_FLAVOR_IS_LOCAL) printf(" B_FLAVOR_IS_LOCAL");
		printf("\n");
		printf("  kinds = 0x%Lx",info->kinds);
		if (info->kinds & B_BUFFER_PRODUCER) printf(" B_BUFFER_PRODUCER");
		if (info->kinds & B_BUFFER_CONSUMER) printf(" B_BUFFER_CONSUMER");
		if (info->kinds & B_TIME_SOURCE) printf(" B_TIME_SOURCE");
		if (info->kinds & B_CONTROLLABLE) printf(" B_CONTROLLABLE");
		if (info->kinds & B_FILE_INTERFACE) printf(" B_FILE_INTERFACE");
		if (info->kinds & B_ENTITY_INTERFACE) printf(" B_ENTITY_INTERFACE");
		if (info->kinds & B_PHYSICAL_INPUT) printf(" B_PHYSICAL_INPUT");
		if (info->kinds & B_PHYSICAL_OUTPUT) printf(" B_PHYSICAL_OUTPUT");
		if (info->kinds & B_SYSTEM_MIXER) printf(" B_SYSTEM_MIXER");
		printf("\n");
		printf("  in_format_count = %ld\n",info->in_format_count);
		printf("  out_format_count = %ld\n",info->out_format_count);

	}
	printf("WantsAutoStart = %s\n",addon->WantsAutoStart() ? "yes" : "no");
	printf("\n");
	
	if (addon->WantsAutoStart()) {
		for (int32 index = 0; ;index++) {
			BMediaNode *outNode;
			int32 outInternalID;
			bool outHasMore;
			rv = addon->AutoStart(index, &outNode, &outInternalID, &outHasMore);
			if (rv == B_OK) {
				printf("started node %ld\n",index);
				rv = mediaroster->RegisterNode(outNode);
				if (rv != B_OK)
					printf("failed to register node %ld\n",index);
				if (!outHasMore)
//					break;
					return;
			} else if (rv == B_MEDIA_ADDON_FAILED && outHasMore) {
				continue;
			} else {
				break;
			}
		}
		return;
	}
	
	
	media_addon_id id;
	id = nextid++;
	printf("adding media add-on %d\n",(int)id);
	filemap->Insert(file_node, id);

finished:
	delete addon;
	unload_add_on(image);
}


void
Application::AddOnRemoved(ino_t file_node)
{	
	media_addon_id id;
	BMediaAddon *addon;
	if (!filemap->Get(file_node,&id)) {
		printf("inode %Ld removed, but no media add-on found\n",file_node);
		return;
	}
	filemap->Remove(file_node);
	printf("removing media add-on %d\n",(int)id);
	if (!addonmap->Get(id,&addon)) {
		printf("no BMediaAddon obejct found\n");
		return;
	}
	delete addon;
	addonmap->Remove(id);
}

void 
Application::WatchDir(BEntry *dir)
{
	BEntry e;
	BDirectory d(dir);
	node_ref nref;
	entry_ref ref;
	while (B_OK == d.GetNextEntry(&e, false)) {
		e.GetRef(&ref);
		e.GetNodeRef(&nref);
		BMessage msg(B_NODE_MONITOR);
		msg.AddInt32("opcode",B_ENTRY_CREATED);
		msg.AddInt32("device",ref.device);
		msg.AddInt64("directory",ref.directory);
		msg.AddInt64("node",nref.node);
		msg.AddString("name",ref.name);
		msg.AddBool("nowait",true);
		MessageReceived(&msg);
	}

	dir->GetNodeRef(&nref); 
	watch_node(&nref,B_WATCH_DIRECTORY,be_app_messenger);
}

void 
Application::MessageReceived(BMessage *msg)
{
	switch (msg->what) 
	{
		case B_NODE_MONITOR:
		{
			switch (msg->FindInt32("opcode")) 
			{
				case B_ENTRY_CREATED:
				{
					const char *name;
					entry_ref ref;
					ino_t node;
					BEntry e;
					BPath p;
					msg->FindString("name", &name);
					msg->FindInt64("node", &node);
					msg->FindInt32("device", &ref.device);
					msg->FindInt64("directory", &ref.directory);
					ref.set_name(name);
					e.SetTo(&ref,false);// build a BEntry for the created file/link/dir
					e.GetPath(&p); 		// get the path to the file/link/dir
					e.SetTo(&ref,true); // travese links to see
					if (e.IsFile()) {		// if it's a link to a file, or a file
						if (false == msg->FindBool("nowait")) {
							// XXX wait 5 seconds if this is a regular notification
							// because the file creation may not be finshed when the
							// notification arrives (very ugly, how can we fix this?)
							// this will also fail if copying takes longer than 5 seconds
							snooze(5000000);
						}
						AddOnAdded(p.Path(),node);
					}
					return;
				}
				case B_ENTRY_REMOVED:
				{
					ino_t node;
					msg->FindInt64("node",&node);
					AddOnRemoved(node);
					return;
				}
				case B_ENTRY_MOVED:
				{
					ino_t from;
					ino_t to;
					msg->FindInt64("from directory", &from);
					msg->FindInt64("to directory", &to);
					if (DirNodeSystem == from || DirNodeUser == from) {
						msg->ReplaceInt32("opcode",B_ENTRY_REMOVED);
						msg->AddInt64("directory",from);
						MessageReceived(msg);
					}
					if (DirNodeSystem == to || DirNodeUser == to) {
						msg->ReplaceInt32("opcode",B_ENTRY_CREATED);
						msg->AddInt64("directory",to);
						msg->AddBool("nowait",true);
						MessageReceived(msg);
					}
					return;
				}
			}
			break;
		}
	}
	printf("Unhandled message:\n");
	msg->PrintToStream();
}

int main()
{
	new Application("application/x-vnd.OpenBeOS-media-addon-server");
	be_app->Run();
	return 0;
}
