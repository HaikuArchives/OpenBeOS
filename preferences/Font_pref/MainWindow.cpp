#ifndef MAIN_WINDOW_H
	
	#include "MainWindow.h"

#endif


MainWindow::MainWindow(BRect frame)
				: BWindow(frame, "Fonts", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE ){
				
	BRect r; 
	BTabView *tabView; 
	BTab *tab; 
	BBox *topLevelView;
	
	r = Bounds(); 
	r.InsetBy(5,5); 
	
	tabView = new BTabView(r, "tab_view"); 
	tabView->SetViewColor(216,216,216,0); 
	
	r = tabView->Bounds(); 
	r.InsetBy(5,5); 
	r.bottom -= tabView->TabHeight(); 
	tab = new BTab(); 
	tabView->AddTab(fontPanel = new FontView(r), tab); 
	tab->SetLabel("Fonts"); 
	/*
	tab = new BTab(); 
	tabView->AddTab(new DestView(r), tab); 
	tab->SetLabel("Obliterate");
	*/
	
	topLevelView = new BBox(Bounds(), "TopLevelView", B_FOLLOW_ALL, B_WILL_DRAW, B_NO_BORDER);
	
	AddChild(tabView);
	AddChild(topLevelView);
	
}

bool MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

void MainWindow::MessageReceived(BMessage *message){

	switch(message->what){
	
		case PLAIN_SIZE_CHANGED_MSG:
		
			updateSize(fontPanel->plainSelectionView);
			break;
		
		case BOLD_SIZE_CHANGED_MSG:
		
			updateSize(fontPanel->boldSelectionView);
			break;
		
		case FIXED_SIZE_CHANGED_MSG:
		
			updateSize(fontPanel->fixedSelectionView);
			break;
		
		case PLAIN_FONT_CHANGED_MSG:
		
			updateFont(fontPanel->plainSelectionView);
			break;
		
		case BOLD_FONT_CHANGED_MSG:
		
			updateFont(fontPanel->boldSelectionView);
			break;
		
		case FIXED_FONT_CHANGED_MSG:
		
			updateFont(fontPanel->fixedSelectionView);
			break;
			
		case PLAIN_STYLE_CHANGED_MSG:
		
			updateStyle(fontPanel->plainSelectionView);
			break;
		
		case BOLD_STYLE_CHANGED_MSG:
		
			updateStyle(fontPanel->boldSelectionView);
			break;
		
		case FIXED_STYLE_CHANGED_MSG:
		
			updateStyle(fontPanel->fixedSelectionView);
			break;
		
		default:
		
			BWindow::MessageReceived(message);
	
	}
	
}

void MainWindow::updateSize(FontSelectionView *theView){

	BFont workingFont;
	
	workingFont = theView->GetTestTextFont();
	workingFont.SetSize(theView->GetSelectedSize());
	theView->SetTestTextFont(&workingFont);
		
}//updateSize

void MainWindow::updateFont(FontSelectionView *theView){

	BFont workingFont;
	font_family updateTo;
	
	theView->UpdateFontSelection();
	workingFont = theView->GetTestTextFont();
	theView->GetSelectedFont(&updateTo);
	workingFont.SetFamilyAndStyle(updateTo, NULL);
	theView->SetTestTextFont(&workingFont);
	
}//updateFont

void MainWindow::updateStyle(FontSelectionView *theView){

	BFont workingFont;
	font_style updateTo;
	font_family update;
	
	theView->UpdateFontSelectionFromStyle();
	workingFont = theView->GetTestTextFont();
	theView->GetSelectedStyle(&updateTo);
	theView->GetSelectedFont(&update);
	workingFont.SetFamilyAndStyle(update, updateTo);
	theView->SetTestTextFont(&workingFont);

}//updateStyle
