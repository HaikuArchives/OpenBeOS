#ifndef MOUSE_SETTINGS_H_
#define MOUSE_SETTINGS_H_

#include <SupportDefs.h>
#include <Screen.h>

typedef enum {
        MOUSE_1_BUTTON = 1,
        MOUSE_2_BUTTON,
        MOUSE_3_BUTTON
} mouse_type;

typedef struct {
        int32           left;
        int32           right;
        int32           middle;
} map_mouse;

typedef struct {
        bool    enabled;        // Acceleration on / off
        int32   accel_factor;   // accel factor: 256 = step by 1, 128 = step by 1/2
        int32   speed;          // speed accelerator (1=1X, 2 = 2x)...
} mouse_accel;

typedef struct {
        mouse_type      type;
        map_mouse       map;
        mouse_accel     accel;
        bigtime_t       click_speed;
} mouse_settings;

class MouseSettings{
public :
	MouseSettings();
	virtual ~MouseSettings();
	BRect WindowPosition() const { return fWindowFrame; }
	void SetWindowPosition(BRect);
	
private:
	static const char 	kMouseSettingsFile[];
	BRect				fWindowFrame;
	BPoint				fcorner;
	mouse_settings		fsettings;
};

#endif