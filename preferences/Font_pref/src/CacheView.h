
#ifndef CACHE_VIEW_H

	#define CACHE_VIEW_H
	
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
			
			CacheView(BRect frame, int minVal, int maxVal, int currVal);
			
		private:
		
			BSlider *screenFCS;
			BSlider *printFCS;
			
	};
	
#endif