#ifndef _BLUE_INVOKER_H
#define	_BLUE_INVOKER_H

#include <BeBuild.h>
#include <app/Messenger.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class BHandler;
class BLooper;
class BMessage;

class BInvoker {
  public:
						BInvoker(); 
						BInvoker(BMessage *message,
							 const BHandler *handler,
							 const BLooper *looper = NULL);
						BInvoker(BMessage *message, BMessenger target);
	virtual				~BInvoker();

	virtual	status_t	SetMessage(BMessage *message);
	BMessage			*Message() const;
	uint32				Command() const;

	virtual status_t	SetTarget(const BHandler *h, const BLooper *loop = NULL);
	virtual status_t	SetTarget(BMessenger messenger);
	bool				IsTargetLocal() const;
	BHandler			*Target(BLooper **looper = NULL) const;
	BMessenger			Messenger() const;

	virtual status_t	SetHandlerForReply(BHandler *handler);
	BHandler			*HandlerForReply() const;

	virtual	status_t	Invoke(BMessage *msg = NULL);
	
	status_t			InvokeNotify(BMessage *msg, uint32 kind = B_CONTROL_INVOKED);
	status_t			SetTimeout(bigtime_t timeout);
	bigtime_t			Timeout() const;

  protected:
	uint32				InvokeKind(bool* notify = NULL);
	void				BeginInvokeNotify(uint32 kind = B_CONTROL_INVOKED);
	void				EndInvokeNotify();

 private:
// to be able to keep binary compatibility
	virtual	void		_ReservedInvoker1();
	virtual	void		_ReservedInvoker2();
	virtual	void		_ReservedInvoker3();

	BMessage			*fMessage;
	BMessenger			fMessenger;
	BHandler			*fReplyTo;
	bigtime_t			fTimeout;
	uint32				fNotifyKind;
	uint32				_reserved[2];			// to be able to keep binary compatibility
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif


#endif /* _INVOKER_H */
