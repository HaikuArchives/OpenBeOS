
#ifndef BUTTON_VIEW_H

	#define BUTTON_VIEW_H
	
	#define RESCAN_FONTS_MSG 'rscn'
	#define RESET_FONTS_MSG 'rset'
	#define REVERT_MSG 'rvrt'
	
	#ifndef _VIEW_H
		
		#include <View.h>
	
	#endif
	
	#ifndef _BUTTON_H
	
		#include <Button.h>
		
	#endif
	
	class ButtonView : public BView{
	
		public:
			
			ButtonView(BRect frame);
			bool RevertState();
			void SetRevertState(bool b);
			
		private:
		
			BButton *revertButton;
			
	};
	
#endif
