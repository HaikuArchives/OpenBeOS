//-----------------------------------------------------------------------------
//
#include <Midi.h>
#include <List.h>
#include <iostream.h>

BMidi::BMidi() {
	_con_list = new BList();
}
BMidi::~BMidi() {
	delete _con_list;
}

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
	return _is_running;
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

}

void BMidi::SprayNoteOff(uchar chan, uchar note, uchar vel,
	uint32 time) const {
    int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			NoteOff(chan,note,vel,time);
	}
}

void BMidi::SprayNoteOn(uchar chan, uchar note, uchar vel,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			NoteOn(chan,note,vel,time);
	}
}

void BMidi::SprayKeyPressure(uchar chan, uchar note, uchar pres,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			KeyPressure(chan,note,pres,time);
	}
}

void BMidi::SprayControlChange(uchar chan, uchar ctrl_num, uchar ctrl_val,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			ControlChange(chan,ctrl_num,ctrl_val,time);
	}
}

void BMidi::SprayProgramChange(uchar chan, uchar prog_num,
	uint32 time) const {                            
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			ProgramChange(chan,prog_num,time);
	}
}

void BMidi::SprayChannelPressure(uchar chan, uchar pres,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			ChannelPressure(chan,pres,time);
	}
}

void BMidi::SprayPitchBend(uchar chan, uchar lsb, uchar msb,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			PitchBend(chan,lsb,msb,time);
	}
}

void BMidi::SpraySystemExclusive(void * data, size_t data_len,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			SystemExclusive(data,data_len);
	}
}

void BMidi::SpraySystemCommon(uchar stat_byte, uchar data1, uchar data2,
	uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			SystemCommon(stat_byte,data1,data2,time);
	}
}

void BMidi::SpraySystemRealTime(uchar stat_byte, uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			SystemRealTime(stat_byte,time);
	}
}

void BMidi::SprayTempoChange(int32 bpm, uint32 time) const {
	int32 num;
	for(int32 i = 0; i < num; i++) {
		((BMidi *)_con_list->ItemAt(i))->
			TempoChange(bpm,time);
	}
}

bool BMidi::KeepRunning() {
	return _keep_running;
}

void BMidi::Run() {
	while(KeepRunning()) {
		snooze(50000);
	}
}

int32 BMidi::_RunThread(void * data) {
	((BMidi *)data)->Run();
}

