/*
	Layer.cpp
		Class used for rendering to the frame buffer. One layer per view on screen and
		also for window decorators
*/
#include "Layer.h"
#include "ServerWindow.h"
#include "Desktop.h"
#include "DisplayDriver.h"
#include <iostream.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG_LAYERS


Layer::Layer(BRect rect, const char *layername, ServerWindow *srvwin,
	int32 viewflags, int32 token)
{
	if(rect.IsValid())
		frame=rect;
	else
	{
#ifdef DEBUG_LAYERS
printf("Invalid BRect: "); rect.PrintToStream();
#endif
		frame.Set(0,0,1,1);
	}

	name=new BString(layername);

	// Layer does not start out as a part of the tree
	parent=NULL;
	uppersibling=NULL;
	lowersibling=NULL;
	topchild=NULL;
	bottomchild=NULL;

	visible=new BRegion(frame);
	invalid=NULL;

	serverwin=srvwin;
	
	view_token=token;

	flags=viewflags;

	hidecount=0;
	is_dirty=false;
	is_updating=false;

	// Because this is not part of the tree, level is negative (thus, irrelevant)
	level=0;
#ifdef DEBUG_LAYERS
	PrintToStream();
#endif
}

Layer::Layer(BRect rect, const char *layername)
{
	// This constructor is used for the top layer in each workspace
	if(rect.IsValid())
		frame=rect;
	else
		frame.Set(0,0,1,1);

	name=new BString(layername);

	// Layer is the root for the workspace
	parent=NULL;
	uppersibling=NULL;
	lowersibling=NULL;
	topchild=NULL;
	bottomchild=NULL;

	visible=new BRegion(frame);
	invalid=NULL;

	serverwin=NULL;
	flags=0;
	hidecount=0;
	is_dirty=false;
	level=0;

	view_token=-1;
#ifdef DEBUG_LAYERS
	PrintToStream();
#endif
}

Layer::~Layer(void)
{
#ifdef DEBUG_LAYERS
	cout << "Layer Destructor for " << name->String() << endl << flush;
#endif
	if(visible!=NULL)
		delete visible;
	if(invalid!=NULL)
		delete invalid;
	delete name;
}

void Layer::AddChild(Layer *layer)
{
	// Adds a layer to the top of the layer's children
#ifdef DEBUG_LAYERS
	cout << "AddChild " << layer->name->String() << endl;
#endif
	
	if(layer->parent!=NULL)
	{
		cout << "ERROR: AddChild(): View already has parent\n" << flush;
		return;
	}
	layer->parent=this;

	if(topchild!=NULL)
	{
		layer->lowersibling=topchild;
		topchild->uppersibling=layer;
	}
	topchild=layer;
}

void Layer::RemoveChild(Layer *layer)
{
	// Remove a layer from the tree
#ifdef DEBUG_LAYERS
	cout << "RemoveChild " << layer->name->String() << endl << flush;
#endif
	
	if(layer->parent==NULL)
	{
		cout << "ERROR: RemoveChild(): View doesn't have a parent\n" << flush;
		return;
	}
	if(layer->parent!=this)
	{
		cout << "ERROR: RemoveChild(): View is not a child of this layer\n" << flush;
		return;
	}

	// Take care of parent
	layer->parent=NULL;
	if(topchild==layer)
		topchild=layer->lowersibling;
	if(bottomchild==layer)
		bottomchild=layer->uppersibling;

	// Take care of siblings
	if(layer->uppersibling!=NULL)
		layer->uppersibling->lowersibling=layer->lowersibling;
	if(layer->lowersibling!=NULL)
		layer->lowersibling->lowersibling=layer->uppersibling;
	layer->SetLevel(0);
	layer->uppersibling=NULL;
	layer->lowersibling=NULL;
}

void Layer::RemoveSelf(void)
{
	// A Layer removes itself from the tree (duh)
#ifdef DEBUG_LAYERS
	cout << "RemoveChild " << name->String() << endl;
#endif
	if(parent==NULL)
	{
		cout << "ERROR: RemoveSelf(): View doesn't have a parent\n" << flush;
		return;
	}
	parent->RemoveChild(this);
}

Layer *Layer::GetChildAt(BPoint pt, bool recursive=false)
{
	// Find out which child gets hit if we click at a certain spot. Returns NULL
	// if there are no visible children or if the click does not hit a child layer
	// If recursive==true, then it will continue to call until it reaches a layer
	// which has no children, i.e. a layer that is at the top of its 'branch' in
	// the layer tree
	
	Layer *child;
	if(recursive)
	{
		for(child=topchild; child!=NULL; child=child->lowersibling)
		{
			if(child->topchild!=NULL)
				child->GetChildAt(pt,true);
			
			if(child->hidecount>0)
				continue;
			
			if(child->frame.Contains(pt))
				return child;
		}
	}
	else
	{
		for(child=topchild; child!=NULL; child=child->lowersibling)
		{
			if(child->hidecount>0)
				continue;
			
			if(child->frame.Contains(pt))
				return child;
		}
	}
	return NULL;
}

void Layer::SetLevel(int32 value)
{
	// Sets hierarchy level of layer and all children
#ifdef DEBUG_LAYERS
	printf("SetLevel %ld\n",value);
#endif
	level=value;

	Layer *lay;

	lay=topchild;

	while(lay!=NULL)
	{
		if(lay->topchild!=NULL)
			lay->topchild->SetLevel(value+1);
		lay=lay->lowersibling;
	}
}

void Layer::PruneTree(void)
{
	// recursively deletes all children (and grandchildren, etc) of the passed layer
	// This is mostly used for server shutdown or deleting a workspace
#ifdef DEBUG_LAYERS
	printf("PruneTree() at level %ld\n", level);
#endif
	Layer *lay,*nextlay;

	lay=topchild;

	while(lay!=NULL)
	{
		if(lay->topchild!=NULL)
			lay->topchild->PruneTree();
		nextlay=lay->lowersibling;
		delete lay;
		lay=nextlay;
	}
	// Man, this thing is short. Elegant, ain't it? :P
}

Layer *Layer::FindLayer(int32 token)
{
	// recursive search for a layer based on its view token
	Layer *lay, *trylay;

	// Search child layers first
	for(lay=topchild; lay!=NULL; lay=lay->lowersibling)
	{
		if(lay->view_token==token)
			return lay;
	}
	
	// Hmmm... not in this layer's children. Try lower descendants
	for(lay=topchild; lay!=NULL; lay=lay->lowersibling)
	{
		trylay=lay->FindLayer(token);
		if(trylay)
			return trylay;
	}
	
	// Well, we got this far in the function, so apparently there is no match to be found
	return NULL;
}

void Layer::Invalidate(BRect rect)
{
	// Make our own section dirty and pass it on to any children, if necessary....
	// YES, WE ARE SHARING DIRT! Mudpies anyone? :D
	is_dirty=true;

	// Crap. Not sure if we need to convert the invalid BRect to our own space
	// because I'm not sure who will be calling this...	I *think* we need to convert
	// it from the parent's space.
	if(invalid)
		invalid->Include(rect);
	else
		invalid=new BRegion(rect);
	
	Layer *lay;
	for(lay=topchild;lay!=NULL; lay=lay->lowersibling)
		lay->Invalidate(rect);
}

void Layer::RequestDraw(void)
{
	if(visible==NULL || hidecount>0)
		return;

	if(serverwin)
	{
		if(invalid==NULL)
			invalid=new BRegion(*visible);
		serverwin->RequestDraw(invalid->Frame());
		delete invalid;
		invalid=NULL;
	}
}

void Layer::ShowLayer(void)
{
	if(hidecount==0)
		return;

	hidecount--;
	if(hidecount==0)
	{
		parent->is_dirty=true;
		is_dirty=true;
	}
	
	Layer *child;
	for(child=topchild; child!=NULL; child=child->lowersibling)
		child->ShowLayer();
}

void Layer::HideLayer(void)
{
	if(hidecount==0)
	{
		parent->is_dirty=true;
		is_dirty=true;
	}
	hidecount++;
	
	Layer *child;
	for(child=topchild; child!=NULL; child=child->lowersibling)
		child->HideLayer();
}

uint32 Layer::CountChildren(void)
{
	uint32 i=0;
	Layer *lay=topchild;
	while(topchild!=NULL)
	{
		lay=lay->lowersibling;
		i++;
	}
	return i;
}

void Layer::PrintToStream(void)
{
	printf("-----------\nLayer %s\n",name->String());
	if(parent)
		printf("Parent: %s (%p)\n",parent->name->String(), parent);
	else
		printf("Parent: NULL\n");
	if(uppersibling)
		printf("Upper sibling: %s (%p)\n",uppersibling->name->String(), uppersibling);
	else
		printf("Upper sibling: NULL\n");
	if(lowersibling)
		printf("Lower sibling: %s (%p)\n",lowersibling->name->String(), lowersibling);
	else
		printf("Lower sibling: NULL\n");
	if(topchild)
		printf("Top child: %s (%p)\n",topchild->name->String(), topchild);
	else
		printf("Top child: NULL\n");
	if(bottomchild)
		printf("Bottom child: %s (%p)\n",bottomchild->name->String(), bottomchild);
	else
		printf("Bottom child: NULL\n");
	printf("Token: %ld\nLevel: %ld\n",view_token, level);
	printf("Hide count: %u\n",hidecount);
	printf("Has invalid areas = %s\n",(is_dirty)?"yes":"no");
	printf("Is updating = %s\n",(is_updating)?"yes":"no");
}

RootLayer::RootLayer(BRect rect, const char *layername, ServerWindow *srvwin, 
	int32 viewflags, int32 token)
	: Layer(rect,layername,srvwin,viewflags,token)
{
}

RootLayer::RootLayer(BRect rect, const char *layername)
	: Layer(rect,layername)
{
}

void RootLayer::RequestDraw(void)
{
	if(!invalid)
		return;
	
	DisplayDriver *driver=get_gfxdriver();
	for(int32 i=0; invalid->CountRects();i++)
	{
		driver->FillRect(invalid->RectAt(i),bgcolor);
	}
	delete invalid;
	invalid=NULL;
}

void RootLayer::SetColor(rgb_color col)
{
	bgcolor=col;
}

rgb_color RootLayer::GetColor(void) const
{	
	return bgcolor;
}
