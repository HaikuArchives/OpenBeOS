#ifndef _KEYBOARDFILE_H_
#define _KEYBOARDFILE_H_

const char WorkspacesPreferences::kKeyboardSettingFile[] = "/boot/home/config/settings/Keyboard_settings";

class KeyboardSettings{
public :
	KeyboardSettings();
	virtual ~KeyboardSettings();
	BRect WindowFrame() const;
	void SetWindowFrame(BRect);
private:
	static const char kKeyboardSettingsFile[];
	BRect fWindowFrame;
};

#endif