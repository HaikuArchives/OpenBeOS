//-----------------------------------------------------------------------------
//
#include <MidiStore.h>
#include <Entry.h>
#include <Path.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "MidiEvent.h"

BMidiStore::BMidiStore() {
	_evt_list = new BList();
}

BMidiStore::~BMidiStore() {
	delete _evt_list;
}

void BMidiStore::NoteOff(uchar chan, uchar note, uchar vel, uint32 time) {
}
    	                 
void BMidiStore::NoteOn(uchar chan, uchar note, uchar velo, uint32 time) {
}

void BMidiStore::KeyPressure(uchar chan, uchar note, uchar pres,
	uint32 time) {
}

void BMidiStore::ControlChange(uchar chan, uchar ctrl_num, uchar ctrl_val,
	uint32 time) {
}
                               
void BMidiStore::ProgramChange(uchar chan, uchar prog_num, uint32 time) {
}

void BMidiStore::ChannelPressure(uchar chan, uchar pres, uint32 time) {                                
}                                 
                                 
void BMidiStore::PitchBend(uchar chan, uchar lsb, uchar msb, uint32 time) {
}

void BMidiStore::SystemExclusive(void * data, size_t data_len, uint32 time) {
}

void BMidiStore::SystemCommon(uchar stat_byte, uchar data1, uchar data2,
	uint32 time) {
}

void BMidiStore::SystemRealTime(uchar stat_byte, uint32 time) {
}

void BMidiStore::TempoChange(int32 bpm, uint32 time) {
}

void BMidiStore::AllNotesOff(bool just_chan, uint32 time) {
}

status_t BMidiStore::Import(const entry_ref * ref) {
	cout.flush();
	BEntry file_entry(ref,true);
	BPath file_path;
	file_entry.GetPath(&file_path);
	ifstream inf(file_path.Path(), ios::in | ios::binary);
	if(!inf) {
		return B_ERROR;
	}
	inf.seekg(0,ios::end);
	uint32 file_len = inf.tellg();
	inf.seekg(ios::beg);
	uint8 * data = new uint8[file_len];
	if(data == NULL) {
		return B_ERROR;
	}
	inf.read(data,file_len);
	inf.close();
	uint8 * d = data;
	if(strncmp((const char *)d,"MThd",4) != 0) {
		return B_BAD_MIDI_DATA;
	}
	d += 4;
	uint32 hdr_len = (uint32)d[0] << 24 | (uint32)d[1] << 16 |
		(uint32)d[2] << 8 | (uint32)d[3];
	if(hdr_len != 6) {
		return B_BAD_MIDI_DATA;
	}
	d += 4;
	uint32 format = (uint32)d[0] << 8 | (uint32)d[1];
	uint32 tracks = (uint32)d[2] << 8 | (uint32)d[3];
	uint32 division = (uint32)d[4] << 8 | (uint32)d[5];
	d += 6;
	status_t ret;
	switch(format) {
	case 0:
		ret = _DecodeFormat0Tracks(d,tracks,file_len-(data-d));
		break;
	case 1:
		return B_ERROR;
	case 2:
		return B_ERROR;
	default:
		return B_BAD_MIDI_DATA;
	}
	return ret;
}

status_t BMidiStore::Export(const entry_ref * ref, int32 format) {
	ofstream ouf(ref->name, ios::out | ios::binary);
	if(!ouf) {
		return B_ERROR;
	}
	return B_OK;
}

void BMidiStore::SortEvents(bool force) {
}

uint32 BMidiStore::CountEvents() const {
	return 0;
}

uint32 BMidiStore::CurrentEvent() const {
	return 0;
}

void BMidiStore::SetCurrentEvent(uint32 event_num) {
}

uint32 BMidiStore::DeltaOfEvent(uint32 event_num) const {
	return 0;
}

uint32 BMidiStore::EventAtDelta(uint32 time) const {
	return 0;
}

uint32 BMidiStore::BeginTime() const {
	return 0;
}

void BMidiStore::SetTempo(int32 bpm) {
}

int32 BMidiStore::Tempo() const {
	return 0;
}

void BMidiStore::Run() {
	while(KeepRunning()) {
		snooze(1000000);
		cout << "s";
		cout.flush();
	}
}

//-----------------------------------------------------------------------------
//
status_t BMidiStore::_DecodeFormat0Tracks(uint8 * data,
	uint16 tracks, uint32 len) {
	cout << "Decoding " << tracks << " tracks" << endl;
	


	return B_OK;
}
