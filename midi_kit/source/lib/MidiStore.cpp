//-----------------------------------------------------------------------------
//
#include <MidiStore.h>
#include <Entry.h>
#include <Path.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "MidiEvent.h"

#define PACK_4_U8_TO_U32(_d1, _d2) \
	_d2 = (uint32)_d1[0] << 24 | (uint32)_d1[1] << 16 | \
		(uint32)_d1[2] << 8 | (uint32)_d1[3];
#define PACK_2_U8_TO_U16(_d1, _d2) \
	_d2 = (uint16)_d1[0] << 8 | (uint16)_d1[1];

BMidiStore::BMidiStore() {
	_evt_list = new BList();
	_tempo = 60;
	_cur_evt = 0;
}

BMidiStore::~BMidiStore() {
	int32 num_items = _evt_list->CountItems();
	for(int i = num_items - 1; i >= 0; i--) {
		delete (BMidiEvent *)_evt_list->RemoveItem(i);
	}
	delete _evt_list;
}

void BMidiStore::NoteOff(uchar chan, uchar note, uchar vel, uint32 time) {
}
    	                 
void BMidiStore::NoteOn(uchar chan, uchar note, uchar vel, uint32 time) {
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
	BEntry file_entry(ref,true);
	BPath file_path;
	file_entry.GetPath(&file_path);
	ifstream inf(file_path.Path(), ios::in | ios::binary);
	if(!inf) {
		return B_ERROR;
	}
	inf.seekg(0,ios::end);
	uint32 file_len = inf.tellg();
	if(file_len < 16) {
		return B_BAD_MIDI_DATA;
	}
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
	uint32 hdr_len;
	PACK_4_U8_TO_U32(d,hdr_len);
	if(hdr_len != 6) {
		return B_BAD_MIDI_DATA;
	}
	d += 4;
	uint32 format;
	uint32 tracks;
	uint32 division;
	PACK_2_U8_TO_U16(d,format);
	d += 2;
	PACK_2_U8_TO_U16(d,tracks);
	d += 2;
	PACK_2_U8_TO_U16(d,division);
	d += 2;
	status_t ret;
	try {
		switch(format) {
		case 0:
			_DecodeFormat0Tracks(d,tracks,file_len-(data-d));
			break;
		case 1:
			_DecodeFormat1Tracks(d,tracks,file_len-(data-d));
			break;
		case 2:
			_DecodeFormat2Tracks(d,tracks,file_len-(data-d));
			break;
		default:
			return B_BAD_MIDI_DATA;
		}
	} catch(status_t err) {
		return err;
	}
	return B_OK;
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
	return _evt_list->CountItems();
}

uint32 BMidiStore::CurrentEvent() const {
	return _cur_evt;
}

void BMidiStore::SetCurrentEvent(uint32 event_num) {
	_cur_evt = event_num;
}

uint32 BMidiStore::DeltaOfEvent(uint32 event_num) const {
	BMidiEvent * e = (BMidiEvent *)_evt_list->ItemAt(event_num);
	return e->time - _start_time;
}

uint32 BMidiStore::EventAtDelta(uint32 time) const {
	return 0;
}

uint32 BMidiStore::BeginTime() const {
	return _start_time;
}

void BMidiStore::SetTempo(int32 bpm) {
	_tempo = bpm;
}

int32 BMidiStore::Tempo() const {
	return _tempo;
}

void BMidiStore::Run() {
	while(KeepRunning()) {
		BMidiEvent * e = (BMidiEvent *)_evt_list->ItemAt(_cur_evt);
		if(e == NULL) {
			return;
		}
		uint32 cur_time = e->time;
		uint32 now = B_NOW;
		while(e->time == cur_time) {
			BMidiEvent::Data & d = e->data;
			switch(e->opcode) {
			case BMidiEvent::OP_NOTE_OFF:
				SprayNoteOff(d.note_off.channel,
					d.note_off.note, d.note_off.velocity,now);
				break;
			case BMidiEvent::OP_NOTE_ON:
				SprayNoteOn(d.note_on.channel,
					d.note_on.note,d.note_on.velocity,now);
				break;
			case BMidiEvent::OP_KEY_PRESSURE:
				SprayKeyPressure(d.key_pressure.channel,
					d.key_pressure.note,d.key_pressure.pressure,now);
				break;
			case BMidiEvent::OP_CONTROL_CHANGE:
				SprayControlChange(d.control_change.channel,
					d.control_change.number,d.control_change.value,now);
				break;
			case BMidiEvent::OP_PROGRAM_CHANGE:
				SprayProgramChange(d.program_change.channel,
					d.program_change.number,now);
				break;
			case BMidiEvent::OP_CHANNEL_PRESSURE:
				SprayChannelPressure(d.channel_pressure.channel,
					d.channel_pressure.pressure,now);
				break;
			case BMidiEvent::OP_PITCH_BEND:
				SprayPitchBend(d.pitch_bend.channel,
					d.pitch_bend.lsb,d.pitch_bend.msb,now);
				break;
			case BMidiEvent::OP_SYSTEM_EXCLUSIVE:
				SpraySystemExclusive(d.system_exclusive.data,
					d.system_exclusive.length,now);
				break;
			case BMidiEvent::OP_SYSTEM_COMMON:
				SpraySystemCommon(d.system_common.status,
					d.system_common.data1,d.system_common.data2,now);
				break;
			case BMidiEvent::OP_SYSTEM_REAL_TIME:
				SpraySystemRealTime(d.system_real_time.status,now);
				break;
			case BMidiEvent::OP_TEMPO_CHANGE:
				SprayTempoChange(d.tempo_change.beats_per_minute,now);
				_tempo = d.tempo_change.beats_per_minute;
				break;
			case BMidiEvent::OP_ALL_NOTES_OFF:
				break;
			default:
				break;
			}
			_cur_evt++;
			e = (BMidiEvent *)_evt_list->ItemAt(_cur_evt);
			if(e == NULL) {
				return;
			}
		}
		uint32 beat_len = 6000000 / _tempo;
		uint32 next_time = e->time;
		snooze((next_time - cur_time) * beat_len);
	}
}

//-----------------------------------------------------------------------------
//
void BMidiStore::_DecodeFormat0Tracks(uint8 * data, uint16 tracks,
	uint32 len) {
	uint8 * last_byte = data + len;
	if(tracks != 1) {
		throw B_BAD_MIDI_DATA;
	}
	uint8 * d = data;
	if(strncmp((const char *)d,"MTrk",4) != 0) {
		throw B_BAD_MIDI_DATA;
	}
	d += 4;
	uint32 trk_len;
	PACK_4_U8_TO_U32(d,trk_len);
	d += 4;
	uint32 time = 0;
	try {
		while(d < last_byte) {
			uint32 evt_dtime = _GetVarLength(&d,last_byte);
			time += evt_dtime;
			BMidiEvent * event = new BMidiEvent();
			bool track_end = _GetEvent(&d,last_byte,event);
			if(track_end) {
				break;
			}
			event->time = time;
			_evt_list->AddItem(event);
		}
	} catch(status_t e) {
		if(e == B_OK) {
			return;
		}
		throw e;
	}
}

void BMidiStore::_DecodeFormat1Tracks(uint8 * data, uint16 tracks,
	uint32 len) {
	return;	
}


void BMidiStore::_DecodeFormat2Tracks(uint8 * data, uint16 tracks,
	uint32 len) {	
	return;
}

bool BMidiStore::_GetEvent(uint8 ** data, uint8 * max_d, BMidiEvent * event) {
#define CHECK_DATA(_d) if((_d) > max_d) throw B_BAD_MIDI_DATA;
#define INC_DATA(_d) CHECK_DATA(_d); d = _d;
	uint8 tmp8;
	uint16 tmp16;
	uint32 tmp32;
	uint8 * d = *data;
	if(d == NULL) {
		throw B_BAD_MIDI_DATA;
	}
	if(d[0] == 0xff) {
		INC_DATA(d+1);
		uint32 len;
		if(d[0] == 0x0) {
			INC_DATA(d+1);
			if(d[0] == 0x00) {
				// Sequence number!
				INC_DATA(d+1);
			} else if(d[0] == 0x02) {
				// Sequence number!
				INC_DATA(d+3);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] > 0x00 && d[0] < 0x0a) {
			INC_DATA(d+1);
			len = _GetVarLength(&d,max_d);
			INC_DATA(d+len);
		} else if(d[0] == 0x2f) {
			INC_DATA(d+1);
			if(d[0] == 0x00) {
				INC_DATA(d+1);
				return true; // End of track.
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x51) {
			INC_DATA(d+1);
			if(d[0] == 0x03) {
				event->opcode = BMidiEvent::OP_TEMPO_CHANGE;
				INC_DATA(d+1);
				CHECK_DATA(d+2);
				uint32 mspb = (uint32)d[0] << 16 |
					(uint32)d[1] << 8 | (uint32)d[2];
				uint32 bpm = 60000000 / mspb;
				event->data.tempo_change.beats_per_minute = bpm;
				INC_DATA(d+3);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x54) {
			INC_DATA(d+1);
			if(d[0] == 0x05) {
				INC_DATA(d+1);
				// SMPTE start time.
				// hour = d[0];
				// minute = d[1];
				// second = d[2];
				// frame = d[3];
				// fraction = d[4];
				INC_DATA(d+5);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x58) {
			INC_DATA(d+1);
			if(d[0] == 0x04) {
				INC_DATA(d+1);
				// Time signature.
				// numerator = d[0];
				// denominator = d[1];
				// midi clocks = d[2];
				// 32nd notes in a quarter = d[3];
				INC_DATA(d+4);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x59) {
			INC_DATA(d+1);
			if(d[0] == 0x02) {
				INC_DATA(d+1);
				// Key signature.
				// sf = d[0];
				// minor = d[1];
				INC_DATA(d+2);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x7f) {
			INC_DATA(d+1);
			uint32 len = _GetVarLength(&d,max_d);
			INC_DATA(d+len);
		} else if(d[0] == 0x20) {
			INC_DATA(d+1);
			if(d[0] == 0x01) {
				INC_DATA(d+2);
			} else {
				throw B_BAD_MIDI_DATA;
			}
		} else if(d[0] == 0x21) {
			INC_DATA(d+1);
			if(d[0] == 0x01) {
				INC_DATA(d+2);
			}
		} else {
			throw B_BAD_MIDI_DATA;
		}
	} else if(d[0] == 0xf0) {
		INC_DATA(d+1);
		// Manufacturer ID = d[0];
		while(d[0] != 0xf7) {
			// Read data eliding the msb.
			INC_DATA(d+1);
		}
	} else if((d[0] & 0xf0) == 0x80) {
		event->opcode = BMidiEvent::OP_NOTE_OFF;
		event->data.note_off.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.note_off.note = d[0];
		INC_DATA(d+1);
		event->data.note_off.velocity = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0x90) {
		event->opcode = BMidiEvent::OP_NOTE_ON;
		event->data.note_on.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.note_on.note = d[0];
		INC_DATA(d+1);
		event->data.note_on.velocity = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0xa0) {
		event->opcode = BMidiEvent::OP_KEY_PRESSURE;
		event->data.key_pressure.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.key_pressure.note = d[0];
		INC_DATA(d+1);
		event->data.key_pressure.pressure = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0xb0) {
		event->opcode = BMidiEvent::OP_CONTROL_CHANGE;
		event->data.control_change.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.control_change.number = d[0];
		INC_DATA(d+1);
		event->data.control_change.value = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0xc0) {
		event->opcode = BMidiEvent::OP_PROGRAM_CHANGE;
		event->data.program_change.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.program_change.number = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0xd0) {
		event->opcode = BMidiEvent::OP_CHANNEL_PRESSURE;
		event->data.channel_pressure.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.channel_pressure.pressure = d[0];
		INC_DATA(d+1);
	} else if((d[0] & 0xf0) == 0xe0) {
		event->opcode = BMidiEvent::OP_PITCH_BEND;
		event->data.pitch_bend.channel = d[0] & 0x0f;
		INC_DATA(d+1);
		event->data.pitch_bend.lsb = d[0];
		INC_DATA(d+1);
		event->data.pitch_bend.msb = d[0];
		INC_DATA(d+1);
	} else {
		cerr << "Unsupported Code:0x" << hex << (int)d[0] << endl;
		INC_DATA(d+1);
		return event;
	}
	*data = d;
	return false;
#undef INC_DATA
#undef CHECK_DATA
}

uint32 BMidiStore::_GetVarLength(uint8 ** data, uint8 * max_d) {
	uint32 val;
	uint8 bytes = 1;
	uint8 byte = 0;
	uint8 * d = *data;
	val = *d;
	d++;
	if(val & 0x80) {
		val &= 0x7f;
		do {
			if(d > max_d) {
				throw B_BAD_MIDI_DATA;
			}
			byte = *d;
			val = (val << 7) + byte & 0x7f;
			bytes++;
			d++;
		} while(byte & 0x80);
	}
	*data = d;
	return val;
}