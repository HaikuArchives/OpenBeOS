//-----------------------------------------------------------------------------
#ifndef _MIDI_STORE_H_
#define _MIDI_STORE_H_

#include <Midi.h>
#include <List.h>

struct entry_ref;
class BMidiEvent;
class BFile;

class BMidiStore : public BMidi {
public:
BMidiStore();
virtual ~BMidiStore();

	virtual void NoteOff(uchar channel, uchar note,
		uchar velocity, uint32 time = B_NOW);
	virtual void NoteOn(uchar channel, uchar note,
		uchar velocity, uint32 time = B_NOW);
	virtual void KeyPressure(uchar channel, uchar note,
		uchar pressure, uint32 time = B_NOW);
	virtual void ControlChange(uchar channel, uchar control_number,
		uchar control_value, uint32 time = B_NOW);
	virtual void ProgramChange(uchar channel, uchar program_number,
		uint32 time = B_NOW);
	virtual void ChannelPressure(uchar channel, uchar pressure,
		uint32 time = B_NOW);
	virtual void PitchBend(uchar channel, uchar lsb,
		uchar msb, uint32 time = B_NOW);
	virtual void SystemExclusive(void * data, size_t data_length,
		uint32 time = B_NOW);
	virtual void SystemCommon(uchar status_byte, uchar data1,
		uchar data2, uint32 time = B_NOW);
	virtual void SystemRealTime(uchar status_byte, uint32 time = B_NOW);
	virtual void TempoChange(int32 beats_per_minute, uint32 time = B_NOW);
	virtual void AllNotesOff(bool just_channel = true, uint32 time = B_NOW);

	status_t Import(const entry_ref * ref);
	status_t Export(const entry_ref * ref, int32 format);

	void SortEvents(bool force = false);
	uint32 CountEvents() const;
	uint32 CurrentEvent() const;
	void SetCurrentEvent(uint32 event_number);
	uint32 DeltaOfEvent(uint32 event_number) const;
	uint32 EventAtDelta(uint32 time) const;
	uint32 BeginTime() const;    
	void SetTempo(int32 beats_per_minute);
	int32 Tempo() const;

private:
	virtual void Run();
	void _RunThread();
	status_t _DecodeFormat0Tracks(uint8 * data, uint16 tracks, uint32 len);
	status_t _EncodeFormat0Tracks(uint8 * data);
	
private:
	BList * _evt_list;
};

#endif _MIDI_STORE_H_
