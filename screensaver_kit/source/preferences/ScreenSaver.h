#ifndef _ScreenSaver_H
#define _ScreenSaver_H
#include <View.h>
#include <Window.h>
#include <Slider.h>
#include <Button.h>
#include <CheckBox.h>
#include <StringView.h>
#include <Picture.h>

static const char *times[]={"30 seconds","1 minute","1 minute 30 seconds","2 minutes","2 minutes 30 seconds",
		  "3 minutes","4 minutes","5 minutes","6 minutes","7 minutes","8 minutes","9 minutes",
		  "10 minutes","15 minutes","20 minutes","25 minutes","30 minutes","40 minutes","50 minutes",
		  "1 hour","1 hour 30 minutes","2 hours","2 hours 30 minutes","3 hours","4 hours","5 hours"};
static const int timeInSeconds[]={30,60,90,120,150,180,240,300,360,420,480,540,600,900,1200,1500,1800,2400,3000,
		  3600,5400,7200,9000,10800,14400,18000};
const int TAB1_CHG=1972;
const int TAB2_CHG=1974;
class ScreenSaver: public BWindow {
public:
  ScreenSaver(void) : BWindow(BRect(50,50,500,385),"OBOS Screen Saver Preferences",B_TITLED_WINDOW,B_ASYNCHRONOUS_CONTROLS)
  {
	SetupForm();
  }
  virtual void MessageReceived(BMessage *message);
  virtual bool QuitRequested(void);
  virtual ~ScreenSaver(void) {};

private:
  void SetupForm(void);
  void setupTab1(void);
  void setupTab2(void);
  void updateStatus(void);
  void loadSettings(BMessage *msg);
  void saveSettings(void);

  int fadeState,noFadeState;
  BView *sampleView;
  BView *tab1,*tab2;
  BTabView *tabView;
  BBox *Box1;
  BListView *ListView1;
  BButton *TestButton;
  BButton *AddButton;
  BBox *EnableScreenSaverBox;
  BSlider *PasswordSlider;
  BSlider *TurnOffSlider;
  BSlider *RunSlider;
  BStringView *StringView1;
  BCheckBox *EnableCheckbox;
  BCheckBox *PasswordCheckbox;
  BCheckBox *TurnOffScreenCheckBox;
  BStringView *TurnOffMinutes;
  BStringView *RunMinutes;
  BStringView *PasswordMinutes;
  BButton *PasswordButton;
  BStringView *FadeNowString;
  BStringView *FadeNowString2;
  BStringView *DontFadeString;
  BStringView *DontFadeString2;
  BPicture samplePicture;
};

#endif // _ScreenSaver_H
