
#ifndef CACHE_VIEW_H

	#define CACHE_VIEW_H
	
	#define PRINT_FCS_UPDATE_MSG 'pfum'
	#define SCREEN_FCS_UPDATE_MSG 'sfum'
	#define PRINT_FCS_MODIFICATION_MSG 'pfmm'
	#define SCREEN_FCS_MODIFICATION_MSG 'sfmm'
	
	#ifndef _VIEW_H
		
		#include <View.h>
	
	#endif
	
	#ifndef _BOX_H
	
		#include <Box.h>
		
	#endif
	#ifndef _SLIDER_H
	
		#include <Slider.h>
	 
	#endif
	#ifndef _STDIO_H
	
		#include <stdio.h>
		
	#endif
	#ifndef _BUTTON_H
	
		#include <Button.h>
		
	#endif
	
	class CacheView : public BView{
	
		public:
			
			CacheView(BRect frame, int minVal, int maxVal, int32 printCurrVal, int32 screenCurrVal);
			void updatePrintFCS(const char* txt);
			void updateScreenFCS(const char* txt);
			int getPrintFCSValue();
			int getScreenFCSValue();
			void revertToOriginal();
			void resetToDefaults();
			
		private:
		
			BSlider *screenFCS;
			BSlider *printFCS;
			int32 origPrintVal;
			int32 origScreenVal;
			
	};
	
#endif
