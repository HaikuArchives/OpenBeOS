#ifndef MIXERNODE
#define MIXERNODE

class MixerNode
	{
	port_id my_message_port;
	
	public:
	MixerNode();
	~MixerNode();
	
	void 		Start();
	void 		Stop();
	void 		Preroll();
	port_id 	GetPortId();
	
	int32		CountOutputs();
	int32		CountInputs();
	media_output	GetOutput( int32 which );
	media_input		GetInput( int32 which );
	};

#endif