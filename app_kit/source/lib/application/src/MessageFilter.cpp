//------------------------------------------------------------------------------
//	MessageFilter.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/MessageFilter.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------
BMessageFilter::BMessageFilter(uint32 inWhat, filter_hook func)
	:	fFiltersAny(false),
		what(inWhat),
		fDelivery(B_ANY_DELIVERY),
		fSource(B_ANY_SOURCE),
		fLooper(NULL),
		fFilterFunction(func)
{
}
//------------------------------------------------------------------------------
BMessageFilter::BMessageFilter(message_delivery delivery, message_source source,
							   filter_hook func)
	:	fFiltersAny(true),
		what(0),
		fDelivery(delivery),
		fSource(source),
		fLooper(NULL),
		fFilterFunction(func)
{
}
//------------------------------------------------------------------------------
BMessageFilter::BMessageFilter(message_delivery delivery, message_source source,
							   uint32 inWhat, filter_hook func)
	:	fFiltersAny(false),
		what(inWhat),
		fDelivery(delivery),
		fSource(source),
		fLooper(NULL),
		fFilterFunction(func)
{
}
//------------------------------------------------------------------------------
BMessageFilter::BMessageFilter(const BMessageFilter& filter)
{
	*this = filter;
}
//------------------------------------------------------------------------------
BMessageFilter::BMessageFilter(const BMessageFilter* filter)
{
	*this = *filter;
}
//------------------------------------------------------------------------------
BMessageFilter::~BMessageFilter()
{
	;
}
//------------------------------------------------------------------------------
BMessageFilter& BMessageFilter::operator=(const BMessageFilter& from)
{
	fFiltersAny			= from.FiltersAnyCommand();
	what				= from.Command();
	fDelivery			= from.MessageDelivery();
	fSource				= from.MessageSource();
	fFilterFunction		= from.FilterFunction();

	SetLooper(from.Looper());

	return *this;
}
//------------------------------------------------------------------------------
filter_result BMessageFilter::Filter(BMessage* message, BHandler** target)
{
	if (fFilterFunction)
	{
		return fFilterFunction(message, target);
	}

	return B_DISPATCH_MESSAGE;
}
//------------------------------------------------------------------------------
message_delivery BMessageFilter::MessageDelivery() const
{
	return fDelivery;
}
//------------------------------------------------------------------------------
message_source BMessageFilter::MessageSource() const
{
	return fSource;
}
//------------------------------------------------------------------------------
uint32 BMessageFilter::Command() const
{
	return what;
}
//------------------------------------------------------------------------------
bool BMessageFilter::FiltersAnyCommand() const
{
	return fFiltersAny;
}
//------------------------------------------------------------------------------
BLooper* BMessageFilter::Looper() const
{
	return fLooper;
}
//------------------------------------------------------------------------------
void BMessageFilter::_ReservedMessageFilter1()
{
	;
}
//------------------------------------------------------------------------------
void BMessageFilter::_ReservedMessageFilter2()
{
	;
}
//------------------------------------------------------------------------------
void BMessageFilter::SetLooper(BLooper* owner)
{
	// TODO: implement locking?
	fLooper = owner;
}
//------------------------------------------------------------------------------
filter_hook BMessageFilter::FilterFunction() const
{
	return fFilterFunction;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */
