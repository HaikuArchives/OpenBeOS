#include "ScreenSaver.h"
#include <ListView.h>
#include <Application.h>
#include <Button.h>
#include <Box.h>
#include <Font.h>
#include <TabView.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <ScrollView.h>
#include <FindDirectory.h>
#include <stdio.h>

void drawPositionalMonitor(BView *view,BRect areaToDrawIn,int state);
BView *drawSampleMonitor(BView *view, BRect area);
  int columns[4]={15,175,195,430};
  int rows[6]={10,130,135,255,270,290};


void ScreenSaver::MessageReceived(BMessage *msg)
{
  switch(msg->what)
  {
	case TAB1_CHG:
		updateStatus();
      	BWindow::MessageReceived(msg);
      	break;
	case TAB2_CHG:
		updateStatus();
      	BWindow::MessageReceived(msg);
      	break;
	case B_QUIT_REQUESTED:
		be_app->PostMessage(B_QUIT_REQUESTED);
      	BWindow::MessageReceived(msg);
      	break;
    default:
      	BWindow::MessageReceived(msg);
      	break;
  }
}

void ScreenSaver::loadSettings(BMessage *msg)
{
}

void ScreenSaver::saveSettings(void)
{
	BMessage foo;
	foo.AddRect("windowframe",Frame());
	foo.AddInt32("windowtab",tabView->Selection());
	foo.AddInt32("timeflags",EnableCheckbox->Value());
	foo.AddInt32("timefade", timeInSeconds[RunSlider->Value()]);
	foo.AddInt32("timestandby", timeInSeconds[TurnOffSlider->Value()]);
	foo.AddInt32("timesuspend", timeInSeconds[TurnOffSlider->Value()]);
	foo.AddInt32("timeoff", timeInSeconds[TurnOffSlider->Value()]);
	foo.AddInt32("cornernow", -1); // UNIMPLEMENTED
	foo.AddInt32("cornernever", -1); // UNIMPLEMENTED
	foo.AddBool("lockenable",PasswordCheckbox->Value());
	foo.AddInt32("lockdelay", timeInSeconds[PasswordSlider->Value()]);
	foo.AddString("lockpassword", ""); // UNIMPLEMENTED
	foo.AddString("lockmethod", "custom"); // ??????????????
		/* > B_MESSAGE_TYPE        "modulesettings_SuperString"
			>  | What=B_OK
			>  | B_BOOL_TYPE           "fade"                   1
			> B_STRING_TYPE         "modulename"             "Lissart"
		*/
	// Pass this message to the loaded screen saver so that it can add its own preferences
	foo.AddString("modulename", ((BStringItem *)(ListView1->ItemAt(ListView1->CurrentSelection(0))))->Text()); 
  BPath path;
  find_directory(B_USER_SETTINGS_DIRECTORY,&path);
  path.Append("OBOS_Screen_Saver",true);
  BFile file(path.Path(),B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
  foo.Flatten(&file);
}

bool ScreenSaver::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}       

void ScreenSaver::updateStatus(void)
{
  DisableUpdates();
  PasswordCheckbox->SetEnabled(EnableCheckbox->Value());
  TurnOffScreenCheckBox->SetEnabled(EnableCheckbox->Value());
  RunSlider->SetEnabled(EnableCheckbox->Value());
  TurnOffSlider->SetEnabled(EnableCheckbox->Value() && TurnOffScreenCheckBox->Value());
  PasswordSlider->SetEnabled(EnableCheckbox->Value() && PasswordCheckbox->Value());
  PasswordButton->SetEnabled(EnableCheckbox->Value() && PasswordCheckbox->Value());
  RunSlider->SetLabel(times[RunSlider->Value()]);
  TurnOffSlider->SetLabel(times[TurnOffSlider->Value()]);
  PasswordSlider->SetLabel(times[PasswordSlider->Value()]);
  sampleView=drawSampleMonitor(tab1,BRect(columns[0],rows[0],columns[1],rows[1]));
  //tab1->DrawPicture(&samplePicture);
  EnableUpdates();
};

void ScreenSaver::SetupForm(void)
{
	BRect r;
	BView *background;
	BTab *tab;
	r = Bounds();

	background=new BView(r,"background",B_FOLLOW_NONE,0);
	background->SetViewColor(216,216,216,0);
	AddChild(background);

	r.InsetBy(0,3);
	tabView = new BTabView(r, "tab_view");
	tabView->SetViewColor(216,216,216,0);
	r = tabView->Bounds();
	r.InsetBy(0,4);
	r.bottom -= tabView->TabHeight();

	tab = new BTab();
	tabView->AddTab(tab2=new BView(r,"Fade",B_FOLLOW_NONE,0), tab);
	tab->SetLabel("Fade");

	tab = new BTab();
	tabView->AddTab(tab1=new BView(r,"Modules",B_FOLLOW_NONE,0), tab);
	tab->SetLabel("Modules");
	background->AddChild(tabView);
	setupTab2();
	setupTab1();
	updateStatus();
}

void commonLookAndFeel(BView *widget,bool isSlider,bool isControl)
	{
  	{rgb_color clr = {216,216,216,255}; widget->SetViewColor(clr);}
	if (isSlider)
		{
		BSlider *slid=dynamic_cast<BSlider *>(widget);
  		{rgb_color clr = {160,160,160,0}; slid->SetBarColor(clr);}
  		slid->SetHashMarks(B_HASH_MARKS_NONE);
  		slid->SetHashMarkCount(0);
  		slid->SetStyle(B_TRIANGLE_THUMB);
  		slid->SetLimitLabels("","");
  		slid->SetLimitLabels(NULL,NULL);
  		slid->SetLabel("0 minutes");
  		slid->SetValue(0);
		slid->SetEnabled(true);
		}
  	{rgb_color clr = {0,0,0,0}; widget->SetHighColor(clr);}
  	widget->SetFlags(B_WILL_DRAW|B_NAVIGABLE);
  	widget->SetResizingMode(B_FOLLOW_NONE);
  	widget->SetFontSize(10);
  	widget->SetFont(be_plain_font);
	if (isControl)
		{
		BControl *wid=dynamic_cast<BControl *>(widget);
		wid->SetEnabled(true);
		}
	}

void addScreenSavers (directory_which dir, BListView *list)
{
  BPath path;
  find_directory(dir,&path);
  path.Append("Screen Savers",true);
  BDirectory ssDir(path.Path());
  BEntry thisSS;
  char thisName[B_FILE_NAME_LENGTH];

  while (B_OK==ssDir.GetNextEntry(&thisSS,true))
  	{
	thisSS.GetName(thisName);
  	list->AddItem(new BStringItem(thisName)); 
  	}
}

void ScreenSaver::setupTab1(void)
{
  {rgb_color clr = {216,216,216,255}; tab1->SetViewColor(clr);}
  tab1->AddChild( Box1 = new BBox(BRect(columns[2],rows[0],columns[3],rows[5]),"Box1"));
  commonLookAndFeel(Box1,false,false);
  Box1->SetLabel("Module settings");
  Box1->SetBorder(B_FANCY_BORDER);

  ListView1 = new BListView(BRect(columns[0],rows[2],columns[1],rows[3]),"ListView1",B_SINGLE_SELECTION_LIST);
  tab1->AddChild(new BScrollView("scroll_list",ListView1,B_FOLLOW_NONE,0,false,true));
  commonLookAndFeel(Box1,false,false);
  {rgb_color clr = {255,255,255,0}; ListView1->SetViewColor(clr);}
  ListView1->SetListType(B_SINGLE_SELECTION_LIST);

  tab1->AddChild( TestButton = new BButton(BRect(columns[0],rows[4],94,rows[5]),"TestButton","Test", new BMessage (TAB1_CHG)));
  commonLookAndFeel(TestButton,false,true);
  TestButton->SetLabel("Test");

  tab1->AddChild( AddButton = new BButton(BRect(97,rows[4],columns[1],rows[5]),"AddButton","Add...", new BMessage (TAB1_CHG)));
  commonLookAndFeel(AddButton,false,true);
  AddButton->SetLabel("Add...");

 // tab1->BeginPicture (&samplePicture);
  sampleView=drawSampleMonitor(tab1,BRect(columns[0],rows[0],columns[1],rows[1]));
  //tab1->EndPicture();
  // -----------------------------------------------------------------------------------------
  // Populate the listview with the screensavers that exist.
  
  addScreenSavers(B_BEOS_ADDONS_DIRECTORY,ListView1);
  addScreenSavers(B_USER_ADDONS_DIRECTORY,ListView1);
} 

void ScreenSaver::setupTab2(void)
{
  font_height stdFontHt;
  be_plain_font->GetHeight(&stdFontHt);  
  int stringHeight=(int)(stdFontHt.ascent+stdFontHt.descent),sliderHeight=30;
  int topEdge;
  {rgb_color clr = {216,216,216,255}; tab2->SetViewColor(clr);}
  tab2->AddChild( EnableScreenSaverBox = new BBox(BRect(11,13,437,280),"EnableScreenSaverBox"));
  commonLookAndFeel(EnableScreenSaverBox,false,false);

  EnableCheckbox = new BCheckBox(BRect(0,0,90,stringHeight),"EnableCheckBox","Enable Screen Saver", new BMessage (TAB2_CHG));
  EnableScreenSaverBox->SetLabel(EnableCheckbox);
  EnableScreenSaverBox->SetBorder(B_FANCY_BORDER);

  // Run Module
  topEdge=26;
  EnableScreenSaverBox->AddChild( StringView1 = new BStringView(BRect(21,topEdge,101,topEdge+stringHeight),"StringView1","Run module"));
  commonLookAndFeel(StringView1,false,false);
  StringView1->SetText("Run module");
  StringView1->SetAlignment(B_ALIGN_LEFT);

  EnableScreenSaverBox->AddChild( RunSlider = new BSlider(BRect(132,topEdge,415,topEdge+sliderHeight),"RunSlider","minutes", new BMessage(TAB2_CHG), 0, 25));
  RunSlider->SetModificationMessage(new BMessage(TAB2_CHG));
  commonLookAndFeel(RunSlider,true,true);
  float w,h;
  RunSlider->GetPreferredSize(&w,&h);
  sliderHeight=(int)h;

  // Turn Off
  topEdge+=sliderHeight;
  EnableScreenSaverBox->AddChild( TurnOffScreenCheckBox = new BCheckBox(BRect(9,topEdge,107,topEdge+stringHeight),"TurnOffScreenCheckBox","Turn off screen",  new BMessage (TAB2_CHG)));
  commonLookAndFeel(TurnOffScreenCheckBox,false,true);
  TurnOffScreenCheckBox->SetLabel("Turn off screen");
  TurnOffScreenCheckBox->SetResizingMode(B_FOLLOW_NONE);

  EnableScreenSaverBox->AddChild( TurnOffSlider = new BSlider(BRect(132,topEdge,415,topEdge+sliderHeight),"TurnOffSlider","", new BMessage(TAB2_CHG), 0, 25));
  TurnOffSlider->SetModificationMessage(new BMessage(TAB2_CHG));
  commonLookAndFeel(TurnOffSlider,true,true);

  // Password
  topEdge+=sliderHeight;
  EnableScreenSaverBox->AddChild( PasswordCheckbox = new BCheckBox(BRect(9,topEdge,108,topEdge+stringHeight),"PasswordCheckbox","Password lock",  new BMessage (TAB2_CHG)));
  commonLookAndFeel(PasswordCheckbox,false,true);
  PasswordCheckbox->SetLabel("Password lock");

  EnableScreenSaverBox->AddChild( PasswordSlider = new BSlider(BRect(132,topEdge,415,topEdge+sliderHeight),"PasswordSlider","", new BMessage(TAB2_CHG), 0, 25));
  PasswordSlider->SetModificationMessage(new BMessage(TAB2_CHG));
  commonLookAndFeel(PasswordSlider,true,true);

  topEdge+=sliderHeight;
  EnableScreenSaverBox->AddChild( PasswordButton = new BButton(BRect(331,topEdge,405,topEdge+25),"PasswordButton","Password...",  new BMessage (TAB2_CHG)));
  commonLookAndFeel(PasswordButton,false,true);
  PasswordButton->SetLabel("Password...");

	// Bottom

//  drawPositionalMonitor(EnableScreenSaverBox,BRect(220,205,280,240) ,fadeState);
//  drawPositionalMonitor(EnableScreenSaverBox,BRect(220,205,280,240) ,noFadeState);

  EnableScreenSaverBox->AddChild( FadeNowString = new BStringView(BRect(85,210,188,222),"FadeNowString","Fade now when"));
  commonLookAndFeel(FadeNowString,false,false);
  FadeNowString->SetText("Fade now when");
  FadeNowString->SetAlignment(B_ALIGN_LEFT);

  EnableScreenSaverBox->AddChild( FadeNowString2 = new BStringView(BRect(85,225,188,237),"FadeNowString2","mouse is here"));
  commonLookAndFeel(FadeNowString2,false,false);
  FadeNowString2->SetText("mouse is here");
  FadeNowString2->SetAlignment(B_ALIGN_LEFT);

  EnableScreenSaverBox->AddChild( DontFadeString = new BStringView(BRect(285,210,382,222),"DontFadeString","Don't fade when"));
  commonLookAndFeel(DontFadeString,false,false);
  DontFadeString->SetText("Don't fade when");
  DontFadeString->SetAlignment(B_ALIGN_LEFT);

  EnableScreenSaverBox->AddChild( DontFadeString2 = new BStringView(BRect(285,225,382,237),"DontFadeString2","mouse is here"));
  commonLookAndFeel(DontFadeString2,false,false);
  DontFadeString2->SetText("mouse is here");
  DontFadeString2->SetAlignment(B_ALIGN_LEFT);
}

