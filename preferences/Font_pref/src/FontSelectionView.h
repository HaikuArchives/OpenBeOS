
#ifndef FONT_SELECTION_VIEW_H

	#define FONT_SELECTION_VIEW_H
	
	#define PLAIN_FONT_SELECTION_VIEW 1
	#define BOLD_FONT_SELECTION_VIEW 2
	#define FIXED_FONT_SELECTION_VIEW 3
	
	#define PLAIN_SIZE_CHANGED_MSG 'plsz'
	#define BOLD_SIZE_CHANGED_MSG 'blsz'
	#define FIXED_SIZE_CHANGED_MSG 'fxsz'
	
	#define PLAIN_FONT_CHANGED_MSG 'plfn'
	#define BOLD_FONT_CHANGED_MSG 'blfn'
	#define FIXED_FONT_CHANGED_MSG 'fxfn'
	
	#define PLAIN_STYLE_CHANGED_MSG 'plst'
	#define BOLD_STYLE_CHANGED_MSG 'blst'
	#define FIXED_STYLE_CHANGED_MSG 'fxst'
	
	#ifndef _VIEW_H
		
		#include <View.h>
	
	#endif
	
	#ifndef _BOX_H
	
		#include <Box.h>
		
	#endif
	
	#ifndef _STRING_VIEW_H
	
		#include <StringView.h>
		
	#endif
	
	#ifndef _POP_UP_MENU_H
	
		#include <PopUpMenu.h>
		
	#endif
	
	#ifndef _MENU_FIELD_H
	
		#include <MenuField.h>
		
	#endif
	
	#ifndef _MENU_ITEM_H
	
		#include <MenuItem.h>
		
	#endif
	#ifndef _STDIO_H
	
		#include <stdio.h>
		
	#endif
	
	class FontSelectionView : public BView{
	
		public:
			
			FontSelectionView(BRect rect, const char *name, int type);
			void SetTestTextFont(BFont *fnt);
			BFont GetTestTextFont();
			float GetSelectedSize();
			void GetSelectedFont(font_family *family);
			void GetSelectedStyle(font_style *style);
			void UpdateFontSelectionFromStyle();
			void UpdateFontSelection();
			void buildMenus();
			void emptyMenus();
			void resetToDefaults();
			
		private:
		
			BStringView *testText;
			BPopUpMenu *fontList;
			BPopUpMenu *sizeList;
			int minSizeIndex;
			int maxSizeIndex;
			uint32 setSizeChangedMessage;
			uint32 setFontChangedMessage;
			uint32 setStyleChangedMessage;
			char typeLabel[30];
			BFont defaultFont;
			BFont workingFont;
			void emptyMenu(BPopUpMenu *m);
			void UpdateFontSelection(BFont *fnt);
			
	
	};
	
#endif
