#ifndef KEYBOARD_SETTINGS_H_
#define KEYBOARD_SETTINGS_H_

#include <Point.h>
#include <Screen.h>

typedef struct {
        bigtime_t       key_repeat_delay;
        int32           key_repeat_rate;
} kb_settings;

class KeyboardSettings{
public :
	KeyboardSettings();
	virtual ~KeyboardSettings();
	BRect WindowPosition() const { return fWindowFrame; }
	int32 KeyboardRepeatRate() const { return fsettings.key_repeat_rate; }
	bigtime_t KeyboardDelayRate() const { return fsettings.key_repeat_delay; }
	void SetWindowPosition(BRect);
	void SetKeyboardRepeatRate(int32);
	void SetKeyboardDelayRate(bigtime_t);
	
private:
	static const char 	kKeyboardSettingsFile[];
	BRect				fWindowFrame;
	BPoint				fcorner;
	kb_settings			fsettings;
};

#endif