#include <view.h>

class previewView : public BView
{ 
	virtual void Draw(); 
	public:	
	BView *previewArea;
}

class mouseAreaView : public BView
{
	virtual void Draw(); 
}
