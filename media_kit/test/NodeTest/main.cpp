#include <Application.h>
#include <MediaRoster.h>
#include "ConsumerNode.h"
#include "ProducerNode.h"
#include "misc.h"

BMediaRoster *roster;
ProducerNode *producer;
ConsumerNode *consumer;
status_t rv;

int main()
{
	out("Basic BBufferProducer, BBufferConsumer, BMediaRoster test\n");
	out("for OpenBeOS by Marcus Overhagen <Marcus@Overhagen.de>\n\n");
	out("Creating BApplication now\n");
	BApplication app("application/x-vnd.OpenBeOS-NodeTest");	

	out("Creating MediaRoster\n");
	roster = BMediaRoster::Roster();
	val(roster);

	out("Creating ProducerNode\n");
	producer = new ProducerNode();
	val(producer);

	out("Creating ConsumerNode\n");
	consumer = new ConsumerNode();
	val(consumer);
	
	out("Registering ProducerNode\n");
	rv = roster->RegisterNode(producer);
	val(rv);

	out("Registering ConsumerNode\n");
	rv = roster->RegisterNode(consumer);
	val(rv);

	media_source source;
	media_destination dest;
	media_format format;
	media_output output;
	media_input input;
	
	media_node_id sourceNodeID;
	media_node_id destinationNodeID;
	media_node sourceNode;
	media_node destinationNode;
	int32 count;
/*
	out("Calling producer->ID()\n");
	sourceNodeID = producer->ID();

	out("Calling consumer->ID()\n");
	destinationNodeID = consumer->ID();

	out("Calling producer->Node()\n");
	sourceNode = producer->Node();

	out("Calling consumer->Node()\n");
	destinationNode = consumer->Node();

	out("Calling GetAllOutputsFor(source)\n");
	rv = roster->GetAllOutputsFor(sourceNode,&output,1,&count);
	val(rv);

	out("Calling GetAllInputsFor(dest)\n");
	rv = roster->GetAllInputsFor(destinationNode,&input,1,&count);
	val(rv);
		
	out("Connecting nodes\n");
	rv = roster->Connect(source, dest, &format, &output, &input);
	val(rv);

	wait();

	out("Disconnecting nodes\n");
	rv = roster->Disconnect(sourceNodeID, source, destinationNodeID, dest);
	val(rv);
*/
	wait();
	
	out("Unregistering ProducerNode\n");
	rv = roster->UnregisterNode(producer);
	val(rv);

	out("Unregistering ConsumerNode\n");
	rv = roster->UnregisterNode(consumer);
	val(rv);

	return 0;
}

