#ifndef KEYBOARD_SETTINGS_H_
#define KEYBOARD_SETTINGS_H_

#include <Point.h>

typedef struct {
        bigtime_t       key_repeat_delay;
        int32           key_repeat_rate;
} kb_settings;

class KeyboardSettings{
public :
	KeyboardSettings();
	virtual ~KeyboardSettings();
private:
	static const char kKeyboardSettingsFile[];
	BPoint		*corner;
	kb_settings	*settings;
};

#endif