//-----------------------------------------------------------------------------
//
#include <Midi.h>
#include <List.h>
#include <iostream.h>
#include "MidiEvent.h"

//-----------------------------------------------------------------------------
//
BMidi::BMidi() {
	_con_list = new BList();
	_is_running = false;
	_is_inflowing = false;
	_inflow_thread_id = spawn_thread(_InflowThread, 
		"BMidi Inflow Thread", B_REAL_TIME_PRIORITY, (void *)this);
	status_t ret = resume_thread(_inflow_thread_id);		
}
BMidi::~BMidi() {
	kill_thread(_inflow_thread_id);
	delete _con_list;
}

//-----------------------------------------------------------------------------
//
void BMidi::NoteOff(uchar chan, uchar note, uchar vel, uint32 time) {
}
void BMidi::NoteOn(uchar chan, uchar note, uchar vel, uint32 time) {
}
void BMidi::KeyPressure(uchar chan, uchar note, uchar pres, uint32 time) {
}
void BMidi::ControlChange(uchar chan, uchar ctrl_num, uchar ctrl_val,
	uint32 time) {
}
void BMidi::ProgramChange(uchar chan, uchar prog_num, uint32 time) {
}
void BMidi::ChannelPressure(uchar chan, uchar pres, uint32 time) {
}
void BMidi::PitchBend(uchar chan, uchar lsb, uchar msb, uint32 time) {
}
void BMidi::SystemExclusive(void * data, size_t data_len, uint32 time) {
}
void BMidi::SystemCommon(uchar stat_byte, uchar data1, uchar data2,
	uint32 time) {
}
void BMidi::SystemRealTime(uchar stat_byte, uint32 time) {
}
void BMidi::TempoChange(int32 bpm, uint32 time) {
}
void BMidi::AllNotesOff(bool just_chan, uint32 time) {
}

//-----------------------------------------------------------------------------
// Public API Functions
status_t BMidi::Start() {
	_keep_running = true;
	_run_thread_id = spawn_thread(_RunThread,
		"BMidi Run Thread", B_REAL_TIME_PRIORITY, (void *)this);
	status_t ret = resume_thread(_run_thread_id);
	if(ret != B_OK) {
		return ret;
	}
	_is_running = true;
	return B_OK;
}

void BMidi::Stop() {
	_keep_running = false;
	status_t exit_value;
	status_t ret;
	ret = wait_for_thread(_run_thread_id,&exit_value);
	_is_running = false;
}
    
bool BMidi::IsRunning() const {
	thread_info info;
	get_thread_info(_run_thread_id,&info);
	if(find_thread("BMidi Run Thread") == _run_thread_id) {
		return true;
	}
	return false;
}

void BMidi::Connect(BMidi * to_object) {
	_con_list->AddItem((void *)to_object);
}

void BMidi::Disconnect(BMidi * from_object) {
	_con_list->RemoveItem((void *)from_object);
}

bool BMidi::IsConnected(BMidi * to_object) const {
	return _con_list->HasItem((void *)to_object);
}

BList * BMidi::Connections() const {
	return _con_list;
}

void BMidi::SnoozeUntil(uint32 time) const {
	snooze_until((uint64)time*1000,B_SYSTEM_TIMEBASE);
}

bool BMidi::KeepRunning() {
	return _keep_running;
}

void BMidi::Run() {
	while(KeepRunning()) {
		snooze(50000);
	}
}

//-----------------------------------------------------------------------------
// Spray Functions
void BMidi::SprayNoteOff(uchar chan, uchar note, uchar vel,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_NOTE_OFF;
	e.data.note_off.channel = chan;
	e.data.note_off.note = note;
	e.data.note_off.velocity = vel;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayNoteOn(uchar chan, uchar note, uchar vel,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_NOTE_ON;
	e.data.note_on.channel = chan;
	e.data.note_on.note = note;
	e.data.note_on.velocity = vel;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayKeyPressure(uchar chan, uchar note, uchar pres,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_NOTE_OFF;
	e.data.key_pressure.channel = chan;
	e.data.key_pressure.note = note;
	e.data.key_pressure.pressure = pres;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayControlChange(uchar chan, uchar ctrl_num, uchar ctrl_val,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_CONTROL_CHANGE;
	e.data.control_change.channel = chan;
	e.data.control_change.number = ctrl_num;
	e.data.control_change.value = ctrl_val;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayProgramChange(uchar chan, uchar prog_num,
	uint32 time) const {                            
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_PROGRAM_CHANGE;
	e.data.program_change.channel = chan;
	e.data.program_change.number = prog_num;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayChannelPressure(uchar chan, uchar pres,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_CHANNEL_PRESSURE;
	e.data.channel_pressure.channel = chan;
	e.data.channel_pressure.pressure = pres;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayPitchBend(uchar chan, uchar lsb, uchar msb,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_PITCH_BEND;
	e.data.pitch_bend.channel = chan;
	e.data.pitch_bend.lsb = lsb;
	e.data.pitch_bend.msb = msb;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SpraySystemExclusive(void * data, size_t data_len,
	uint32 time) const {
	// Should this data be duplicated!!??
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_SYSTEM_EXCLUSIVE;
	e.data.system_exclusive.data = (uint8 *)data;
	e.data.system_exclusive.length = data_len;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SpraySystemCommon(uchar stat_byte, uchar data1, uchar data2,
	uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_SYSTEM_COMMON;
	e.data.system_common.status = stat_byte;
	e.data.system_common.data1 = data1;
	e.data.system_common.data2 = data2;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SpraySystemRealTime(uchar stat_byte, uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_SYSTEM_REAL_TIME;
	e.data.system_real_time.status = stat_byte;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

void BMidi::SprayTempoChange(int32 bpm, uint32 time) const {
	BMidiEvent e;
	e.time = time;
	e.opcode = BMidiEvent::OP_TEMPO_CHANGE;
	e.data.tempo_change.beats_per_minute = bpm;
	send_data(_inflow_thread_id, 0, (void *)&e, sizeof(BMidiEvent));
}

//-----------------------------------------------------------------------------
// The Inflow Thread
void BMidi::_Inflow() const {
	thread_id sender_id;
	BMidiEvent e;
	while(true) {
		int32 code = receive_data(&sender_id,(void *)&e,sizeof(BMidiEvent));
		//lock
		int32 num_connections = _con_list->CountItems();
		switch(e.opcode) {
		case BMidiEvent::OP_NONE:
		case BMidiEvent::OP_TRACK_END:
		case BMidiEvent::OP_ALL_NOTES_OFF:
			break;
		case BMidiEvent::OP_NOTE_OFF:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					NoteOff(e.data.note_off.channel,
						e.data.note_off.note,
						e.data.note_off.velocity,
						e.time);
			}
			break;
		case BMidiEvent::OP_NOTE_ON:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					NoteOff(e.data.note_on.channel,
						e.data.note_on.note,
						e.data.note_on.velocity,
						e.time);
			}
			break;
		case BMidiEvent::OP_KEY_PRESSURE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					KeyPressure(e.data.key_pressure.channel,
						e.data.key_pressure.note,
						e.data.key_pressure.pressure,
						e.time);
			}
			break;
		case BMidiEvent::OP_CONTROL_CHANGE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					ControlChange(e.data.control_change.channel,
						e.data.control_change.number,
						e.data.control_change.value,
						e.time);
			}
			break;
		case BMidiEvent::OP_PROGRAM_CHANGE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					ProgramChange(e.data.program_change.channel,
						e.data.program_change.number,
						e.time);
			}
			break;
		case BMidiEvent::OP_CHANNEL_PRESSURE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					ChannelPressure(e.data.channel_pressure.channel,
						e.data.channel_pressure.pressure,
						e.time);
			}
			break;
		case BMidiEvent::OP_PITCH_BEND:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					PitchBend(e.data.pitch_bend.channel,
						e.data.pitch_bend.lsb,
						e.data.pitch_bend.msb,
						e.time);
			}
			break;
		case BMidiEvent::OP_SYSTEM_EXCLUSIVE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					SystemExclusive(e.data.system_exclusive.data,
						e.data.system_exclusive.length,
						e.time);
			}
			break;
		case BMidiEvent::OP_SYSTEM_COMMON:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					SystemCommon(e.data.system_common.status,
						e.data.system_common.data1,
						e.data.system_common.data2,
						e.time);
			}
			break;
		case BMidiEvent::OP_SYSTEM_REAL_TIME:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					SystemRealTime(e.data.system_real_time.status, e.time);
			}
			break;
		case BMidiEvent::OP_TEMPO_CHANGE:
			for(int32 i = 0; i < num_connections; i++) {
				((BMidi *)_con_list->ItemAt(i))->
					TempoChange(e.data.tempo_change.beats_per_minute, e.time);
			}
			break;
		default:
			break;
		}
		//unlock
	}
}

int32 BMidi::_RunThread(void * data) {
	((BMidi *)data)->Run();
}

int32 BMidi::_InflowThread(void * data) {
	((BMidi *)data)->_Inflow();
}
