#ifndef _ScreenSaver_H
#define _ScreenSaver_H
#include <View.h>
#include <Window.h>
#include <Slider.h>
#include <Button.h>
#include <CheckBox.h>
#include <StringView.h>
#include <Picture.h>
#include "Constants.h"
#include "pwWindow.h"

class mouseAreaView;

class ScreenSaver: public BWindow {
public:
  ScreenSaver(void) : BWindow(BRect(50,50,500,385),"OBOS Screen Saver Preferences",B_TITLED_WINDOW,B_ASYNCHRONOUS_CONTROLS)
  {
	pwWin=NULL;
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
  mouseAreaView *fadeNow,*fadeNever;
  pwWindow *pwWin;
  BMessenger *pwMessenger;
};

#endif // _ScreenSaver_H
