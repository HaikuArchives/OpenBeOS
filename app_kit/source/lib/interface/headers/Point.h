#ifndef	_POINT_H
#define	_POINT_H

#include <BeBuild.h>
#include "support/SupportDefs.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class BRect;

class BPoint {
  public:
	float x;
	float y;

	BPoint();
	BPoint(float X, float Y);
	BPoint(const BPoint &p);
		
	BPoint	&operator=(const BPoint &p);
	void	Set(float X, float Y);

	void	ConstrainTo(BRect r);
	void	PrintToStream() const;
			
	BPoint	operator+(const BPoint &p) const;
	BPoint	operator-(const BPoint &p) const;
	BPoint&	operator+=(const BPoint &p);
	BPoint&	operator-=(const BPoint &p);

	bool	operator!=(const BPoint &p) const;
	bool	operator==(const BPoint &p) const;
};

extern const BPoint B_ORIGIN;	// returns (0,0)

inline BPoint::BPoint()
{
	x = y = 0;
}

inline BPoint::BPoint(float X, float Y)
{
	x = X;
	y = Y;
}

inline BPoint::BPoint(const BPoint& pt)
{
	x = pt.x;
	y = pt.y;
}

inline BPoint &BPoint::operator=(const BPoint& from)
{
	x = from.x;
	y = from.y;
	return *this;
}

inline void BPoint::Set(float X, float Y)
{
	x = X;
	y = Y;
}


#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif
