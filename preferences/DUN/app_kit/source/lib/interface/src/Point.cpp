#include <stdio.h>
#include <BeBuild.h>
#include <math.h>
#include "../../support/headers/SupportDefs.h"
#include "../headers/Point.h"
#include "../headers/Rect.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

void BPoint::ConstrainTo(BRect r)
{
	x = max(min(x,r.right),r.left);
	y = max(min(y,r.bottom),r.top);
}

void BPoint::PrintToStream() const
{
	printf("BPoint(x:%.0f, y:%.0f)\n", x, y);
}
			
BPoint	BPoint::operator+(const BPoint &p) const
{
	return (BPoint(x+p.x, y+p.y));
}

BPoint	BPoint::operator-(const BPoint &p) const
{
	return (BPoint(x-p.x, y-p.y));
}

BPoint&	BPoint::operator+=(const BPoint &p)
{
	x+=p.x; y+=p.y;
	return *this;
}

BPoint&	BPoint::operator-=(const BPoint &p)
{
	x-=p.x; y-=p.y;
	return *this;
}

bool BPoint::operator!=(const BPoint &p) const
{
	return( x!=p.x || y!=p.y );
}

bool BPoint::operator==(const BPoint &p) const
{
	return( x==p.x && y==p.y );
}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
