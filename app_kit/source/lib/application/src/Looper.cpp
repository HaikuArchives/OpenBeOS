//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		Looper.cpp
//	Author(s):		Erik Jaesler (erik@cgsoftware.com)
//					DarkWyrm (bpmagic@columbus.rr.com)
//	Description:	BLooper class spawns a thread that runs a message loop.
//------------------------------------------------------------------------------

/**
	@note	Although I'm implementing "by the book" for now, I would like to
			refactor sLooperList and all of the functions that operate on it
			into their own class in the BPrivate namespace.

			Also considering adding the thread priority when archiving.

			
 */

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <Looper.h>
#include <Message.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "TokenSpace.h"

// Local Defines ---------------------------------------------------------------
#define FILTER_LIST_BLOCK_SIZE	5

// Globals ---------------------------------------------------------------------
typedef bool (*find_loop_pred)(_loop_data_* data, void* data);
_loop_data_* find_loop_data(_loop_data_* begin, _loop_data_* end,
							find_loop_pred, void* data);
bool looper_by_port_pred(_loop_data_* looper, void* data);
bool looper_by_tid_pred(_loop_data_* looper, void* data);
bool looper_by_name_pred(_loop_data_* looper, void* data);
bool looper_pred(_loop_data_* looper, void* data);
bool empty_slot_pred(_loop_data_* looper, void* data);
bool copy_list_pred(_loop_data_* looper, void* data);

uint32			BLooper::sLooperID = B_ERROR;
uint32			BLooper::sLooperListSize = 0;
uint32			BLooper::sLooperCount = 0;
_loop_data_*	BLooper::sLooperList = NULL;
BLocker			BLooper::sLooperListLock;
team_id			BLooper::sTeamID = B_ERROR;

//------------------------------------------------------------------------------
BLooper::BLooper(const char* name, int32 priority, int32 port_capacity)
	:	BHandler(name), fMsgPort(-1)
{
	InitData(name, priority, port_capacity);
}
//------------------------------------------------------------------------------
BLooper::~BLooper()
{
	kill_thread(fTaskID);
	delete fQueue;
	delete_sem(fLockSem);
	delete_port(fMsgPort);

	UnlockFully();

	BAutolock ListLock(sLooperListLock);
	RemoveHandler(this);
	RemoveLooper(this);

	// Clean up our filters
	SetCommonFilterList(NULL);
}
//------------------------------------------------------------------------------
BLooper::BLooper(BMessage* data)
	:	BHandler(data)
{
	int32 portCap;
	if (data->FindInt32("_port_cap", &portCap) != B_OK)
	{
		portCap = B_LOOPER_PORT_DEFAULT_CAPACITY;
	}

	InitData(Name(), B_NORMAL_PRIORITY, portCap);
}
//------------------------------------------------------------------------------
BArchivable* BLooper::Instantiate(BMessage* data)
{
	if (validate_instantiation_data(data))
	{
		return new BLooper(data);
	}

	return NULL;
}
//------------------------------------------------------------------------------
status_t BLooper::Archive(BMessage* data, bool deep) const
{
	status_t err = BHandler::Archive(data, deep);
	if (!err)
	{
		port_info info;
		err = get_port_info(fMsgPort, &info);
		if (!err)
		{
			err = data->AddInt32("_port_cap", info.capacity);
		}
	}

	return err;
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(uint32 command)
{
	BMessage Message(command);
	return _PostMessage(&Message, this, NULL);
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(BMessage* message)
{
	return _PostMessage(message, this, NULL);
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(uint32 command, BHandler* handler,
							  BHandler* reply_to)
{
	BMessage Message(command);
	return _PostMessage(&Message, handler, reply_to);
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(BMessage* message, BHandler* handler,
							  BHandler* reply_to)
{
	return _PostMessage(message, handler, reply_to);
}
//------------------------------------------------------------------------------
void BLooper::DispatchMessage(BMessage* message, BHandler* handler)
{
/**
	@note	Initially, DispatchMessage() was locking the looper, calling the
			filtering API, determining whether to use fPreferred or not, and
			deleting the message.  A look at the BeBook, however, reveals that
			all this function does is handle its own B_QUIT_REQUESTED messages
			and pass everything else to handler->MessageReceived().  Clearly the
			rest must be happening in task_looper().  This makes a lot of sense
			because otherwise every derived class would have to figure out when
			to use fPreferred, handle the locking and filtering and delete the
			message.  Even if the BeBook didn't say as much, it would make total
			sense to hoist that functionality out of here and into task_looper().
 */
	switch (message->what)
	{
		case _QUIT_:
		{
			// Can't call Quit() to do this, because of the slight chance
			// another thread with have us locked between now and then.
			fTerminating = true;
			delete this;
			break;
		}

		case B_QUIT_REQUESTED:
		{
			if (handler == this)
			{
				do_quit_requested(message);
				break;
			}
			else
			{
				// fall through
			}
		}

		default:
		{
			handler->MessageReceived(message);
			break;
		}
	}
}
//------------------------------------------------------------------------------
void BLooper::MessageReceived(BMessage* msg)
{
	// TODO: verify
	// The BeBook says this "simply calls the inherited function. ...the BLooper
	// implementation does nothing of importance."  Which is not the same as
	// saying it does nothing.  Investigate.
	BHandler::MessageReceived(msg);
}
//------------------------------------------------------------------------------
BMessage* BLooper::CurrentMessage() const
{
	return fLastMessage;
}
//------------------------------------------------------------------------------
BMessage* BLooper::DetachCurrentMessage()
{
	Lock();
	BMessage* msg = fLastMessage;
	fLastMessage = NULL;
	fQueue.RemoveMessage(msg);
	Unlock();
	return msg;
}
//------------------------------------------------------------------------------
BMessageQueue* BLooper::MessageQueue() const
{
	return fQueue;
}
//------------------------------------------------------------------------------
bool BLooper::IsMessageWaiting() const
{
	if (!fQueue->IsEmpty())
	{
		return true;
	}

	int32 count = port_count(fMsgPort);

	return count != 0; // count > 0;
}
//------------------------------------------------------------------------------
void BLooper::AddHandler(BHandler* handler)
{
	AssertLocked();

	if (handler->Looper() == NULL)
	{
		fHandlers.AddItem(handler);
		handler->SetNextHandler(this);
	}
}
//------------------------------------------------------------------------------
bool BLooper::RemoveHandler(BHandler* handler)
{
	AssertLocked();

	// TODO: test
	// Need to ensure this algo reflects what actually happens
	if (handler->Looper() == this && fHandlers.RemoveItem(handler))
	{
		handler->SetNextHandler(NULL);
		return true;
	}

	return false;
}
//------------------------------------------------------------------------------
int32 BLooper::CountHandlers() const
{
	AssertLocked();

	return fHandlers.CountItems();
}
//------------------------------------------------------------------------------
BHandler* BLooper::HandlerAt(int32 index) const
{
	AssertLocked();

	return (BHandler*)fHandlers.ItemAt(index);
}
//------------------------------------------------------------------------------
int32 BLooper::IndexOf(BHandler* handler) const
{
	AssertLocked();

	// TODO: test
	// ensure the B_ERROR gets returned if the handler isn't in the list
	return fHandlers.IndexOf(handler);
}
//------------------------------------------------------------------------------
BHandler* BLooper::PreferredHandler() const
{
	return fPreferred;
}
//------------------------------------------------------------------------------
void BLooper::SetPreferredHandler(BHandler* handler)
{
	fPreferred = (handler && handler->Looper() == this) ? handler : NULL;
}
//------------------------------------------------------------------------------
thread_id BLooper::Run()
{
	AssertLocked();

	if (fRunCalled)
	{
		// Not allowed to call Run() more than once
		// TODO: test
		// find out what message is actually here
		debugger("");
	}

	fTaskID = spawn_thread(_task0_, name, priority, this);

	if (fTaskID == B_NO_MORE_THREADS || fTaskID == B_NO_MEMORY)
	{
		return fTaskID;
	}

	if (fMsgPort == B_NO_MORE_PORTS || fMsgPort == B_BAD_VALUE)
	{
		return fMsgPort;
	}

	fRunCalled = true;
	Unlock();
	status_t err = resume_thread(fTaskID);
	if (err)
	{
		return err;
	}

	return fTaskID;
}
//------------------------------------------------------------------------------
void BLooper::Quit()
{
	if (!IsLocked())
	{
		const char* name = Name();
		if (!name)
		{
			name = "no-name";
		}
		printf("ERROR - you must Lock a looper before calling Quit(), "
			   "team=%d, looper=%s", Team(), name);
	}

	if (!fRunCalled || find_thread(NULL) == fTaskID)
	{
		fTerminating = true;
		delete this;
	}
	else
	{
		// As with sem in _Lock(), we need to cache this here in case the looper
		// disappears before we get to the wait_for_thread() below
		thread_id tid = Thread();

		// As per the BeBook, if we've been called by a thread other than
		// our own, the rest of the message queue has to get processed.  So
		// we put this in the queue, and when it shows up, we'll call Quit()
		// from our own thread.
		// A little testing with BMessageFilter shows _QUIT_ is being used here.
		// I got suspicious when my test QuitRequested() wasn't getting called
		// when Quit() was invoked from another thread.  Makes a nice proof that
		// this is how it's handled, too.
		status_t err;
		err = PostMessage(_QUIT_);

		// There's a possibility that PostMessage() will return B_WILL_BLOCK
		// because the port is full, so we'll wait a bit and re-post until
		// we won't block.
		while (err == B_WILL_BLOCK)
		{
			// TODO: test this value; it may be too short
			snooze(10000);
			PostMessage(_QUIT_);
		}

		// Also as per the BeBook, we have to wait until the looper is done
		// processing any remaining messages.
		int32 temp;
		do
		{
			err = wait_for_thread(tid, &temp);
		} while (err == B_INTERRUPTED);
	}
}
//------------------------------------------------------------------------------
bool BLooper::QuitRequested()
{
	return true;
}
//------------------------------------------------------------------------------
bool BLooper::Lock()
{
	// Defer to global _Lock(); see notes there
	return _Lock(this, -1, B_INFINITE_TIMEOUT) == B_OK;
}
//------------------------------------------------------------------------------
void BLooper::Unlock()
{
	//	Make sure we're locked to begin with
	AssertLocked();

	//	Decrement fOwnerCount
	--fOwnerCount;
	//	Check to see if the owner still wants a lock
	if (fOwnerCount > 0)
	{
		//	Set fOwner to invalid thread_id (< 0)
		fOwner = -1;

		//	Decrement requested lock count (using fAtomicCount for this)
		--fAtomicCount;

		//	Check if anyone is waiting for a lock
		if (fAtomicCount > 0)
		{
			//	release the lock
			release_sem(fLockSem);
		}
	}
}
//------------------------------------------------------------------------------
bool BLooper::IsLocked() const
{
	// We have to lock the list for the call to IsLooperValid().  Has the side
	// effect of not letting the looper get deleted while we're here.
	BAutolock ListLock(sLooperListLock);

	if (!ListLock.IsLocked())
	{
		// If we can't lock the list, our semaphore is probably toast
		return false;
	}

	if (!IsLooperValid(this))
	{
		// The looper is gone, so of course it's not locked
		return false;
	}

	// Got this from Jeremy's BLocker implementation
	return find_thread(NULL) == fOwner;
}
//------------------------------------------------------------------------------
status_t BLooper::LockWithTimeout(bigtime_t timeout)
{
	return _Lock(this, -1, B_INFINITE_TIMEOUT);
}
//------------------------------------------------------------------------------
thread_id BLooper::Thread() const
{
	return fTaskID;
}
//------------------------------------------------------------------------------
team_id BLooper::Team() const
{
	return sTeamID;
}
//------------------------------------------------------------------------------
BLooper* BLooper::LooperForThread(thread_id tid)
{
	BAutolock ListLock(sLooperListLock);
	if (ListLock.IsLocked())
	{
		_loop_data_* result = find_loop_data(sLooperList,
											 sLooperList + sLooperCount,
											 looper_by_tid_pred, (void*)tid);
		if (result)
		{
			return result->looper;
		}
	}

	return NULL;
}
//------------------------------------------------------------------------------
thread_id BLooper::LockingThread() const
{
	return fOwner;
}
//------------------------------------------------------------------------------
int32 BLooper::CountLocks() const
{
	return fOwnerCount;
}
//------------------------------------------------------------------------------
int32 BLooper::CountLockRequests() const
{
	return fAtomicCount;
}
//------------------------------------------------------------------------------
sem_id BLooper::Sem() const
{
	return fLockSem;
}
//------------------------------------------------------------------------------
BHandler* BLooper::ResolveSpecifier(BMessage* msg, int32 index,
									BMessage* specifier, int32 form,
									const char* property)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
status_t BLooper::GetSupportedSuites(BMessage* data)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::AddCommonFilter(BMessageFilter* filter)
{
	AssertLocked();
	if (!fCommonFilters)
	{
		fCommonFilters = new BList(FILTER_LIST_BLOCK_SIZE);
	}
	fCommonFilters->AddItem(filter);
}
//------------------------------------------------------------------------------
bool BLooper::RemoveCommonFilter(BMessageFilter* filter)
{
	AssertLocked();
	return fCommonFilters->RemoveItem(filter);
}
//------------------------------------------------------------------------------
void BLooper::SetCommonFilterList(BList* filters)
{
	AssertLocked();
	if (fCommonFilters)
	{
		for (int32 i = 0; i < fCommonFilters->CountItems(); ++i)
		{
			delete fCommonFilters->ItemAt(i);
		}
		fCommonFilters->MakeEmpty();
	}

	// Per the BeBook, we take ownership of the list
	fCommonFilters = filters;
}
//------------------------------------------------------------------------------
BList* BLooper::CommonFilterList() const
{
	return fCommonFilters;
}
//------------------------------------------------------------------------------
status_t BLooper::Perform(perform_code d, void* arg)
{
	// This is sort of what we're doing for this function everywhere
	return B_ERROR;
}
//------------------------------------------------------------------------------
BMessage* BLooper::MessageFromPort(bigtime_t timeout)
{
	return ReadMessageFromPort(timeout);
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper1()
{
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper2()
{
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper3()
{
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper4()
{
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper5()
{
}
//------------------------------------------------------------------------------
void BLooper::_ReservedLooper6()
{
}
//------------------------------------------------------------------------------
BLooper::BLooper(const BLooper&)
{
	// Copy construction not allowed
}
//------------------------------------------------------------------------------
BLooper& BLooper::operator=(const BLooper& )
{
	// Looper copying not allowed
}
//------------------------------------------------------------------------------
BLooper::BLooper(int32 priority, port_id port, const char* name)
{
	// This must be a legacy constructor
	fMsgPort = port;
	InitData(name, priority, B_LOOPER_PORT_DEFAULT_CAPACITY);
}
//------------------------------------------------------------------------------
status_t BLooper::_PostMessage(BMessage* msg, BHandler* handler,
							   BHandler* reply_to)
{
	BAutolock ListLock(sLooperListLock)
	if (!ListLock.IsLocked())
	{
		return B_BAD_VALUE;
	}

	if (!IsLooperValid(this))
	{
		return B_BAD_VALUE;
	}

	// Does handler belong to this looper?
	if (handler && handler->Looper() != this)
	{
		return B_MISMATCHED_VALUES;
	}

	status_t err = B_OK;

	BMessenger Messenger(handler, this, &err);

	if (!err)
	{
		err = Messenger.SendMessage(msg, reply_to);
	}

	return err;
}
//------------------------------------------------------------------------------
status_t BLooper::_Lock(BLooper* loop, port_id port, bigtime_t timeout)
{
/**
	@note	The assumption I'm under here is that since we can get the port of
			the BLooper directly from the BLooper itself, the port parameter is
			for identifying BLoopers by port_id when a pointer to the BLooper in
			question is not available.  So this function has two modes:
				o When loop != NULL, use it directly
				o When loop == NULL and port is valid, use the port_id to get
				  the looper
			I scoured the docs to find out what constitutes a valid port_id to
			no avail.  Since create_port uses the standard error values in its
			returned port_id, I'll assume that anything less than zero is a safe
			bet as an *invalid* port_id.  I'm guessing that, like thread and
			semaphore ids, anything >= zero is valid.  So, the short version of
			this reads: if you don't want to find by port_id, make port = -1.

			Another assumption I'm making is that Lock() and LockWithTimeout()
			are covers for this function.  If it turns out that we don't really
			need this function, I may refactor this code into LockWithTimeout()
			and have Lock() call it instead.  This function could then be
			removed.
 */

	//	Check params (loop, port)
	if (!loop && port < 0)
	{
		return B_BAD_VALUE;
	}

	// forward declared so I can use BAutolock on sLooperListLock
	thread_id curThread;
	sem_id sem;

/**
	@note	We lock the looper list at the start of the lock operation to
			prevent the looper getting removed from the list while we're
			doing list operations.  Also ensures that the looper doesn't
			get deleted here (since ~BLooper() has to lock the list as
			well to remove itself).
 */
	{
		BAutolock ListLock(sLooperListLock);
		if (!ListLock.IsLocked())
		{
			// If we can't lock, the semaphore is probably
			// gone, which leaves us in no-man's land
			return B_BAD_VALUE;
		}
	
		//	Look up looper by port_id, if necessary
		if (!loop)
		{
			loop = LooperForPort(port);
			if (!loop)
			{
				return B_BAD_VALUE;
			}
		}
		else
		{
			//	Check looper validity
			if (!IsLooperValid(loop))
			{
				return B_BAD_VALUE;
			}
		}
	
		//	Check for nested lock attempt
		thread_id curThread = find_thread(NULL);
		if (curThread == loop->fOwner)
		{
			//	Bump fOwnerCount
			++loop->fOwnerCount;
			return B_OK;
		}
	
		//	Cache the semaphore
		sem_id sem = loop->fLockSem;
	
		//	Validate the semaphore
		if (sem < 0)
		{
			return B_BAD_VALUE;
		}
	
		//	Bump the requested lock count (using fAtomicLock for this)
		++loop->fAtomicLock;

		// sLooperListLock automatically released here
	}

/**
	@note	We have to operate with the looper list unlocked during semaphore
			acquisition so that the rest of the application doesn't have to
			wait for this lock to happen.  This is why we cached fLockSem
			earlier -- with the list unlocked, the looper might get deleted
			right out from under us.  This is also why we use a raw semaphore
			instead of the easier-to-deal-with BLocker; you can't cache a
			BLocker.
 */
	//	acquire the lock
	status_t err;
	do
	{
		err = acquire_sem_etc(sem, 1, B_RELATIVE_TIMEOUT, timeout);
	} while (err == B_INTERRUPTED);

	if (!err)
	{
		//		Assign current thread to fOwner
		loop->fOwner = curThread;
		//		Reset fOwnerCount to 1
		loop->fOwner = 1;
	}

	return err;
}
//------------------------------------------------------------------------------
status_t BLooper::_LockComplete(BLooper* loop, int32 old, thread_id this_tid,
								sem_id sem, bigtime_t timeout)
{
	// What is this for?  Hope I'm not missing something conceptually here ...
}
//------------------------------------------------------------------------------
void BLooper::InitData()
{
	fOwner = B_ERROR;
	fRunCalled = false;
	fQueue = new BMessageQueue();
	fCommonFilters = NULL;
	fPreferred = NULL;
	fTaskID = B_ERROR;
	fTerminating = false;

	if (sTeamID == -1)
	{
		thread_info info;
		get_thread_info(fTaskID,&info);
		sTeamID = info.team;
	}

	BAutoLock ListLock(sLooperListLock);
	AddLooper(this);
	AddHandler(this);
	Lock();
}
//------------------------------------------------------------------------------
void BLooper::InitData(const char* name, int32 priority, int32 port_capacity)
{
	fLockSem = create_sem(1, name);

	if (fMsgPort >= 0)
	{
		fMsgPort = create_port(port_capacity, name ? name : "LooperPort");
	}

	InitData();
}
//------------------------------------------------------------------------------
void BLooper::AddMessage(BMessage* msg)
{
	// TODO: implement
	// Why is this here?
}
//------------------------------------------------------------------------------
void BLooper::_AddMessagePriv(BMessage* msg)
{
	// TODO: implement
	// No, really; why the hell is this here??
}
//------------------------------------------------------------------------------
status_t BLooper::_task0_(void* arg)
{
	BLooper* obj = (BLooper*)arg;

	if (obj->Lock())
	{
		obj->task_looper();
		obj->fTerminating = true;
		delete obj;
	}

	return B_OK;
}
//------------------------------------------------------------------------------
void* BLooper::ReadRawFromPort(int32* msgcode, bigtime_t tout)
{
	int8* msgbuffer = NULL;
	ssize_t buffersize;
	ssize_t bytesread;

	if (time == B_INFINITE_TIMEOUT)
	{
		buffersize = port_buffer_size(fMsgPort);
	}
	else
	{
		buffersize = port_buffer_size_etc(fMsgPort, 0, time);
		if (buffersize == B_TIMED_OUT || buffersize == B_BAD_PORT_ID ||
			buffersize == B_WOULD_BLOCK)
		{
			return NULL;
		}
	}

	if (buffersize > fMsgBufferSize)
	{
		fMsgBufferSize = buffersize;
		fMsgBuffer = new int8[fMsgBufferSize];
	}

	if (time == B_INFINITE_TIMEOUT)
	{
		bytesread = read_port(fMsgPort, msgcode, msgbuffer, buffersize);
	}
	else
	{
		bytesread = read_port_etc(fMsgPort, msgcode, fMsgBuffer, fMsgBufferSize,
								  B_TIMEOUT, tout);
	}

	return msgbuffer;
}
//------------------------------------------------------------------------------
BMessage* BLooper::ReadMessageFromPort(bigtime_t tout)
{
	int32 msgcode;
	BMessage* bmsg;

	void* msgbuffer = ReadRawFromPort(&msgcode, tout);

	bmsg = ConvertToMessage(msgbuffer, msgcode);
	if (buffersize > 0)
	{
		delete msgbuffer;
	}

	return bmsg;
}
//------------------------------------------------------------------------------
BMessage* BLooper::ConvertToMessage(void* raw, int32 code)
{
	BMessage* bmsg = new BMessage(msgcode);

	if (raw != NULL)
	{
		bmsg->Unflatten((const char*)raw);
	}

	return bmsg;
}
//------------------------------------------------------------------------------
void BLooper::task_looper()
{
	//	Check that looper is locked (should be)
	AssertLocked();
	//	Unlock the looper
	Unlock();

	//	loop:
	while (true)
	{
		//	Are we terminating?
		if (fTerminating)
		{
			//	Bail out
			return;
		}

		//	Read from message port (how do we determine what the timeout is?)
		BMessage* msg = MessageFromPort();

		//	Did we get a message?
		if (msg)
		{
			//	Add to queue
			fQueue->AddMessage(msg);
		}

		//	Get message count from port
		int32 msgCount = port_count(fMsgPort);
		for (int32 i = 0; i < msgCount; ++i)
		{
			//	Read 'count' messages from port (so we will not block)
			//	We use zero as our timeout since we know there is stuff there
			msg = MessageFromPort(0);
			//	Add messages to queue
			fQueue->AddMessage(msg);
		}

		//	loop:
		while (true)
		{
			//	Lock the looper
			Lock();

			//	Get next message from queue (assign to fLastMessage)
			fLastMessage = fQueue->NextMessage();
			if (fLastMessage)
			{
				//	Get the target handler
				//	Use BMessage friend functions to determine if we are using the
				//	preferred handler, or if a target has been specified
				BHandler* handler;
				if (_use_preferred_target_(fLastMessage))
				{
					handler = fPreferred;
				}
				else
				{
					/**
						@note	Here is where all the token stuff starts to
								make sense.  How, exactly, do we determine
								what the target BHandler is?  If we look at
								BMessage, we see an int32 field, fTarget.
								Amazingly, we happen to have a global mapping
								of BHandler pointers to int32s!
					 */
					 gDefaultTokens->GetToken(_get_message_target_(fLastMessage),
					 						  B_HANDLER_TOKEN,
					 						  (void**)&handler);
				}

				if (!handler)
				{
					handler = this;
				}

				//	Is this a scripting message? (BMessage::HasSpecifiers())
				if (fLastMessage->HasSpecifiers())
				{
					int32 index = 0;
					// Make sure the current specifier is kosher
					if (GetCurrentSpecifier(&index) == B_OK)
					{
						handler = resolve_specifier(handler, fLastMessage);
					}
				}
				
				if (handler)
				{
					//	Do filtering
					handler = top_level_filter(fLastMessage, handler);
					if (handler && handler->Looper() == this)
					{
						DispatchMessage(fLastMessage, handler);
					}
				}

				//	Unlock the looper
				Unlock();

				//	Delete the current message (fLastMessage)
				delete fLastMessage;
				fLastMessage = NULL;

				//	Unlock the queue if necessary; can't add anything if
				//	another thread has it locked
				while (fQueue->IsLocked())
				{
					fQueue->Unlock();
				}

				//	Are we terminating?
				if (fTerminating)
				{
					//	Bail out
					return;
				}

				//	Are any messages on the port?
				if (port_count(fMsgPort) > 0)
				{
					//	Do outer loop
					break;
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
void BLooper::do_quit_requested(BMessage* msg)
{
/**
	@note	I couldn't figure out why do_quit_requested() was necessary; why not
			just call Quit()?  Then, while writing the PostMessage() code, I
			realized that the sender of the B_QUIT_REQUESTED message just might
			be waiting for a reply.  A quick test, and yes, we get a reply
			which consists of:
				what: B_REPLY
				"result" (bool) return of QuitRequested()
				"thread" (int32) the looper's thread id

			While Quit() could use fLastMessage, it makes more sense that
			do_quit_requested() would handle it since it gets passed the
			message.
 */

	bool isQuitting = QuitRequested();

	if (msg->IsSourceWaiting())
	{
		BMessage ReplyMsg(B_REPLY);
		ReplyMsg.AddBool("result", isQuitting);
		ReplyMsg.AddInt32("thread", fTaskID);
		msg->SendReply(&ReplyMsg);
	}

	if (isQuitting)
	{
		Quit();
	}
}
//------------------------------------------------------------------------------
bool BLooper::AssertLocked() const
{
	if (!IsLocked())
	{
		debugger("looper must be locked before proceeding\n");
		return false;
	}

	return true;
}
//------------------------------------------------------------------------------
BHandler* BLooper::top_level_filter(BMessage* msg, BHandler* t)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BHandler* BLooper::handler_only_filter(BMessage* msg, BHandler* t)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BHandler* BLooper::apply_filters(BList* list, BMessage* msg, BHandler* target)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::check_lock()
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BHandler* BLooper::resolve_specifier(BHandler* target, BMessage* msg)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::UnlockFully()
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::AddLooper(BLooper* l)
{
	if (sLooperListLock.IsLocked())
	{
#if defined(CHECK_ADD_LOOPER)
		// First see if it's already been added
		if (!IsLooperValid(l))
#endif
		{
			_loop_data_* result = find_loop_data(sLooperList,
												 sLooperList + sLooperCount,
												 empty_slot_pred, NULL);
			if (!result)
			{
				// No empty slots; time to expand
				if (sLooperCount == sLooperListSize)
				{
					// Allocate the expanded list
					_loop_data_* temp =
						new _loop_data_[sLooperListSize + DATA_BLOCK_SIZE];
					if (!temp)
					{
						// Not good
						debugger("unable to allocate looper list");
						return;
					}

					// Transfer the existing data
					memcpy(temp, sLooperList,
						   sizeof (_loop_data*) * sLooperListSize);
					delete[] sLooperList;
					sLooperList = temp;
					sLooperListSize += DATA_BLOCK_SIZE;
				}

				// Whether we expanded or not, the "new" one will be at the end
				result = sLooperList[sLooperCount];
			}

			result->looper = l;
			++sLooperCount;
		}
	}
}
//------------------------------------------------------------------------------
bool BLooper::IsLooperValid(const BLooper* l)
{
	if (sLooperListLock.IsLocked())
	{
		return find_loop_data(sLooperList, sLooperList + sLooperCount,
							  looper_pred, (void*)l);
	}

	return false;
}
//------------------------------------------------------------------------------
void BLooper::RemoveLooper(BLooper* l)
{
	if (sLooperListLock.IsLocked())
	{
		_loop_data_* result = find_loop_data(sLooperList,
											 sLooperList + sLooperCount,
											 looper_pred, l);
		if (result)
		{
			result->looper = NULL;
			--sLooperCount;
		}

		// Nothing left?  Clean up; the app is probably exiting anyway
		if (sLooperCount == 0)
		{
			delete[] sLooperList;
			sLooperList = NULL;
		}
	}
}
//------------------------------------------------------------------------------
void BLooper::GetLooperList(BList* list)
{
	Autolock ListLock(sLooperListLock);
	if (ListLock.IsLocked())
	{
		find_loop_data(sLooperList, sLooperList + sLooperCount,
					   copy_list_pred, (void*)list);
	}
}
//------------------------------------------------------------------------------
BLooper* BLooper::LooperForName(const char* name)
{
	if (sLooperListLock.IsLocked())
	{
		_loop_data_* result = find_loop_data(sLooperList,
											 sLooperList + sLooperCount,
											 looper_by_name_pred, (void*)name);
		if (result)
		{
			return result->looper;
		}
	}

	return NULL;
}
//------------------------------------------------------------------------------
BLooper* BLooper::LooperForPort(port_id port)
{
	if (sLooperListLock.IsLocked())
	{
		_loop_data_* result = find_loop_data(sLooperList,
											 sLooperList + sLooperCount,
											 looper_by_port_pred, (void*)port);
		if (result)
		{
			return result->looper;
		}
	}

	return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
_loop_data_* find_loop_data(_loop_data_* begin, _loop_data_* end,
							find_loop_pred predicate, void* data)
{
	while (begin && begin != end)
	{
		if (begin->looper)
		{
			if (predicate(begin, data))
			{
				return begin;
			}
		}
		++begin;
	}

	return NULL;
}
//------------------------------------------------------------------------------
bool looper_by_port_pred(_loop_data_* looper, void *data)
{
	return looper->looper.fMsgPort == (port_id)data;
}
//------------------------------------------------------------------------------
bool looper_by_tid_pred(_loop_data_* looper, void *data)
{
	return looper->looper.fTaskID == (thread_id)data;
}
//------------------------------------------------------------------------------
bool looper_by_name_pred(_loop_data_* looper, void *data)
{
	return strcmp(looper->looper->Name(), (const char*)data) == 0;
}
//------------------------------------------------------------------------------
bool looper_pred(_loop_data_* looper, void *data)
{
	return looper->looper == (BLooper*)data;
}
//------------------------------------------------------------------------------
bool empty_slot_pred(_loop_data_* looper, void*)
{
	return looper->looper != NULL;
}
//------------------------------------------------------------------------------
bool copy_list_pred(_loop_data_ *looper, void* data)
{
	BList* List = (BList*)data;
	if (List && looper->looper)
	{
		List->AddItem(looper->looper);
	}

	// Ride this train to the end
	return false;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

