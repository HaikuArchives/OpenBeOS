#ifndef _LAYER_H_
#define _LAYER_H_

#include <Rect.h>
#include <Region.h>
#include <List.h>
#include <String.h>

class ServerWindow;

class Layer
{
public:
	Layer(BRect rect, const char *layername, ServerWindow *srvwin,
		int32 viewflags, int32 token);
	Layer(BRect rect, const char *layername);
	virtual ~Layer(void);

	void ShowLayer(void);
	void HideLayer(void);
	Layer *FindLayer(int32 token);
	void MoveLayer(BPoint delta, bool include_children=true);
	void AddChild(Layer *child);
	void RemoveChild(Layer *child);
	void RemoveSelf(void);
	Layer *GetChildAt(BPoint pt);

	void RebuildRegions(bool include_children=true);
	void PrintToStream(void);
	void PruneTree(void);
	void SetLevel(int32 value);
	void Invalidate(BRect rect);
	void RequestDraw(void);

	BRect ConvertToParent(BRect rect);
	BPoint ConvertToParent(BPoint point);
	BRect ConvertFromParent(BRect rect);
	BPoint ConvertFromParent(BPoint point);
	BRect ConvertToTop(BRect rect);
	BPoint ConvertToTop(BPoint point);
	BRect ConvertFromTop(BRect rect);
	BPoint ConvertFromTop(BPoint point);

	BRect frame;

	Layer	*parent,
			*uppersibling,
			*lowersibling,
			*topchild,
			*bottomchild;

	BRegion *visible,
			*invalid;

	ServerWindow *serverwin;

	BString *name;	
	int32 view_token;	// identifier for the corresponding view
	int32 level;		// how far layer is from root
	int32 flags;		// view flags
	uint8 hidecount;
	bool is_dirty;		// true if we need to redraw
	bool is_updating;
};

extern BLocker *layerlock;
extern BList *layerlist;
extern Layer *rootlayer;
#endif