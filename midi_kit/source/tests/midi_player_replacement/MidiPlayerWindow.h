#ifndef MIDI_PLAYER_WINDOW_H
#define MIDI_PLAYER_WINDOW_H

#include "MidiPlayerView.h"

#include <Window.h>
#include <MidiKit.h>
#include <FilePanel.h>

//----------------------------------------------------------

#define FILE			'file'
#define BEOS_SYNTH		'besy'
#define BEOS_SYNTH_FILE	'besf'
#define MIDIPORT		'mipo'

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------

class MidiPlayerWindow : public BWindow
{
public:
	MidiPlayerWindow(BPoint start);
	~MidiPlayerWindow(void);
	
	virtual bool QuitRequested(void);
	virtual void FrameMoved(BPoint origin);
	virtual	void MessageReceived(BMessage *msg);
	
	virtual void Disconnect(void);
	virtual void Connect(void);
	
private:
	MidiPlayerView *fStandartView;

	BMidi *fMidiInput;
	BMidi *fMidiOutput;
	BMidi *fMidiView;
	BMidi *fMidiDelay;
	
	uint32 fInputType;
	uint32 fOutputType;
	uint32 fViewType;

	BFilePanel *fInputFilePanel;
	BFilePanel *fOutputFilePanel;
	
	entry_ref fOutputFile;
	
	uint32 fInputCurrentEvent;
	uint32 fOutputCurrentEvent;
};

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
#endif
