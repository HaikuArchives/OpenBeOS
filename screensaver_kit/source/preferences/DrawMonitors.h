#include <View.h>

class previewView : public BView
{ 
	public:	
	virtual void Draw(BRect update); 
	previewView(BRect frame, const char *name) : BView (frame,name,B_FOLLOW_NONE,B_WILL_DRAW) { SetViewColor(216,216,216); }
	BView *previewArea;
};

enum arrowDirection {UPLEFT,UPRIGHT,DOWNLEFT,DOWNRIGHT,NONE};

class mouseAreaView : public BView
{
	public:	
	mouseAreaView(BRect frame, const char *name) : BView (frame,name,B_FOLLOW_NONE,B_WILL_DRAW) 
		{ SetViewColor(216,216,216); 
		currentDirection=NONE;}

	virtual void Draw(BRect update); 
	virtual void MouseUp(BPoint point);
	void DrawArrow(void);
	inline int getDirection(void) {return ((int)currentDirection);}
	void setDirection(int direction) {currentDirection=(arrowDirection)direction;Draw(BRect (0,0,100,100));}
	private:
	BRect screenArea;
	arrowDirection currentDirection;
};
