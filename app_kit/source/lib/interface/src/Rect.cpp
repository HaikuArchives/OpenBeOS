// BRect
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#include <stdio.h>
#include "interface/Rect.h"
#include "interface/Point.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// InsetBy
BRect& BRect::InsetBySelf(BPoint p){
	this->InsetBy(p);
	return *this;
}

BRect& BRect::InsetBySelf(float dx, float dy){
	this->InsetBy(dx, dy);
	return *this;
}

BRect	BRect::InsetByCopy(BPoint p){
	BRect copy(*this);
	copy.InsetBy(p);
	return copy;
}

BRect	BRect::InsetByCopy(float dx, float dy){
	BRect copy(*this);
	copy.InsetBy(dx, dy);
	return copy;
}

// OffsetBy
BRect& BRect::OffsetBySelf(BPoint p){
	this->OffsetBy(p);
	return *this;
}

BRect& BRect::OffsetBySelf(float dx, float dy){
	this->OffsetBy(dx, dy);
	return *this;
}

BRect	BRect::OffsetByCopy(BPoint p){
	BRect copy(*this);
	copy.OffsetBy(p);
	return copy;
}

BRect	BRect::OffsetByCopy(float dx, float dy){
	BRect copy(*this);
	copy.OffsetBy(dx, dy);
	return copy;
}

// OffsetTo
BRect& BRect::OffsetToSelf(BPoint p){
	this->OffsetTo(p);
	return *this;
}

BRect& BRect::OffsetToSelf(float dx, float dy){
	this->OffsetTo(dx, dy);
	return *this;
}

BRect	BRect::OffsetToCopy(BPoint p){
	BRect copy(*this);
	copy.OffsetTo(p);
	return copy;
}

BRect	BRect::OffsetToCopy(float dx, float dy){
	BRect copy(*this);
	copy.OffsetTo(dx, dy);
	return copy;
}

void	BRect::PrintToStream() const{
	printf("(l:%.1f t:%.1f r:%.1f b:%.1f)\n", left, top, right, bottom);
}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
