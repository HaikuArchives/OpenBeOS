// BInvoker
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#include <stdio.h>
#include "app/Invoker.h"
#include <app/Messenger.h>
#include <app/Handler.h>
#include <app/Message.h>
#include <app/Application.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BInvoker::BInvoker() : fMessenger( be_app )
{
	fMessage = NULL;
	fReplyTo = NULL;
	fTimeout = B_INFINITE_TIMEOUT;
}

BInvoker::BInvoker(BMessage *message, const BHandler *handler, const BLooper *looper = NULL)
{
	fMessage = message;
	fMessenger = BMessenger( handler, looper );
	fReplyTo = NULL;
	fTimeout = B_INFINITE_TIMEOUT;
}

BInvoker::BInvoker(BMessage *message, BMessenger target)
{
	fMessage = message;
	fMessenger = BMessenger(target);
	fReplyTo = NULL;
	fTimeout = B_INFINITE_TIMEOUT;
}

BInvoker::~BInvoker(){
	delete fMessage;
}

status_t BInvoker::SetMessage(BMessage *message)
{
	delete fMessage;
	fMessage = message;
	return B_OK;
}

BMessage *BInvoker::Message() const
{
	return fMessage;
}

uint32 BInvoker::Command() const
{
	if (fMessage)
		return fMessage->what;
	else
		return (int32)NULL;
}

status_t BInvoker::SetTarget(const BHandler *handler, const BLooper *looper = NULL)
{
	fMessenger = BMessenger( handler, looper );
	return fMessenger.IsValid();
}

status_t BInvoker::SetTarget(BMessenger messenger)
{
	fMessenger = messenger;
	return fMessenger.IsValid();
}

bool BInvoker::IsTargetLocal() const
{
	return fMessenger.IsTargetLocal();
}

BHandler *BInvoker::Target(BLooper **looper = NULL) const
{
	return fMessenger.Target(looper);
}

BMessenger BInvoker::Messenger() const
{
	return fMessenger;
}

status_t BInvoker::SetHandlerForReply(BHandler *handler)
{
	fReplyTo = handler;
	return B_OK;
}

BHandler *BInvoker::HandlerForReply() const
{
	return fReplyTo;
}

status_t BInvoker::Invoke(BMessage *msg = NULL)
{
	return InvokeNotify( msg );
}

status_t BInvoker::InvokeNotify(BMessage *msg, uint32 kind = B_CONTROL_INVOKED)
{
	fNotifyKind = kind;

	BMessage clone(kind); 
	status_t err = B_BAD_VALUE;
	
	if (!msg && fNotifyKind == B_CONTROL_INVOKED){
		msg = fMessage;
	}

	if (!msg){
		if (!Target()->IsWatched())
			return err; 
	}else
		clone = *msg;

	clone.AddInt64("when", system_time()); 
	clone.AddPointer("source", this); 

	if (msg)
		err = fMessenger.SendMessage( &clone, fReplyTo, fTimeout);
	
	// Also send invocation to any observers of this handler. 
	Target()->SendNotices(kind, &clone);
	return err;
}

status_t BInvoker::SetTimeout(bigtime_t timeout)
{
	fTimeout = timeout;
	return B_OK;
}

bigtime_t BInvoker::Timeout() const
{
	return fTimeout;
}

uint32 BInvoker::InvokeKind(bool* notify = NULL)
{
	if (fNotifyKind == B_CONTROL_INVOKED)
		*notify = false;
	else
		*notify = true;
	return fNotifyKind;
}

void BInvoker::BeginInvokeNotify(uint32 kind = B_CONTROL_INVOKED)
{
	fNotifyKind = kind;
}

void BInvoker::EndInvokeNotify()
{
	fNotifyKind = B_CONTROL_INVOKED;
}

void BInvoker::_ReservedInvoker1()	{}
void BInvoker::_ReservedInvoker2()	{}
void BInvoker::_ReservedInvoker3()	{}

BInvoker::BInvoker(const BInvoker &){}

BInvoker &BInvoker::operator=(const BInvoker &)
{
	return *this;
}


#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
