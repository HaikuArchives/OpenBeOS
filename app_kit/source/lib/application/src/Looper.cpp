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
 */

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <Looper.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

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

uint32			BLooper::sLooperID = -1;
uint32			BLooper::sLooperListSize = 0;
uint32			BLooper::sLooperCount = 0;
_loop_data_*	BLooper::sLooperList = NULL;
BLocker			BLooper::sLooperListLock;
team_id			BLooper::sTeamID = -1;

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
	delete fLock;
	delete_port(fMsgPort);

	UnlockFully();

	BAutolock ListLock(sLooperListLock);
	RemoveLooper(this);
}
//------------------------------------------------------------------------------
BLooper::BLooper(BMessage* data)
	:	BHandler(data)
{
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
	fQueue->Lock()
	fQueue->AddMessage(new BMessage(command));
	fQueue->Unlock();

	return B_OK;
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(BMessage* message)
{
	fQueue->Lock();
	BMessage* msg = new BMessage();
	*msg = *message;
	
	fQueue->AddMessage(msg);
	fQueue->Unlock();
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(uint32 command, BHandler* handler,
							  BHandler* reply_to)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
status_t BLooper::PostMessage(BMessage* message, BHandler* handler,
							  BHandler* reply_to)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::DispatchMessage(BMessage* message, BHandler* handler)
{
	if (handler == this && message->what == B_QUIT_REQUESTED)
	{
		Quit();
	}

	Lock();	
	
	if (FilterCurrentMessage())
	{
		if (handler == NULL)
		{
			fPreferred->MessageReceived(message);
			Unlock();
			return;
		}
	
		int32 handle_index = fHandlers.IndexOf(handler);
	
		// We only tell the handler to process it if it belongs
		// to our looper
		if (handle_index >= 0)
		{
			handler->MessageReceived(message);
		}
		else
		{
			fPreferred->MessageReceived(message);
		}
		delete message;
	}
	Unlock();
}
//------------------------------------------------------------------------------
void BLooper::MessageReceived(BMessage* msg)
{
	// TODO: implement
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
	BMessage *msg=fLastMessage;
	fLastMessage = NULL;
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
	return fQueue->IsEmpty() ? false : true;
}
//------------------------------------------------------------------------------
void BLooper::AddHandler(BHandler* handler)
{
	if(fLock->IsLocked()==false)
	{
		// TODO: implement
		//(new BAlert("OpenBeOS","Looper must be locked before calling AddHandler","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return;
	}
	fHandlers.AddItem(handler);
}
//------------------------------------------------------------------------------
bool BLooper::RemoveHandler(BHandler* handler)
{
	if(fLock->IsLocked()==false)
	{
		// TODO: implement
		//(new BAlert("OpenBeOS","Looper must be locked before calling RemoveHandler","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return false;
	}
	return(fHandlers.RemoveItem(handler));
}
//------------------------------------------------------------------------------
int32 BLooper::CountHandlers() const
{
	if(fLock->IsLocked()==false)
	{
		// TODO: implement
		//(new BAlert("OpenBeOS","Looper must be locked before calling CountHandlers","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return -1;
	}
	return(fHandlers.CountItems());
}
//------------------------------------------------------------------------------
BHandler* BLooper::HandlerAt(int32 index) const
{
	if(fLock->IsLocked()==false)
	{
		//TODO: implement
		//(new BAlert("OpenBeOS","Looper must be locked before calling HandlerAt","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return NULL;
	}
	return((OBHandler*)fHandlers.ItemAt(index));
}
//------------------------------------------------------------------------------
int32 BLooper::IndexOf(BHandler* handler) const
{
	if(fLock->IsLocked() == false)
	{
		//TODO: implement
		//(new BAlert("OpenBeOS","Looper must be locked before calling IndexOf","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return -1;
	}
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
	fPreferred = (handler != NULL) ? handler : (BHandler*)this;
}
//------------------------------------------------------------------------------
thread_id BLooper::Run()
{
	if (fTaskID == B_NO_MORE_THREADS || fTaskID == B_NO_MEMORY)
	{
		return fTaskID;
	}

	if (fMsgPort == B_NO_MORE_PORTS || fMsgPort == B_BAD_VALUE)
	{
		return fMsgPort;
	}

	fRunCalled = true;
	fLock->Unlock();
	resume_thread(fTaskID);

	return fTaskID;
}
//------------------------------------------------------------------------------
void BLooper::Quit()
{
	if (QuitRequested() == false)
	{
		return;
	}

	if (fLock->IsLocked() == false)
	{
		// TODO: fix
		(new BAlert("OpenBeOS","Looper must be locked before quitting","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT))->Go();
		return;
	}

	delete this;
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
		// If we can't lock, our semaphore is probably toast
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
	// TODO: implement
}
//------------------------------------------------------------------------------
bool BLooper::RemoveCommonFilter(BMessageFilter* filter)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::SetCommonFilterList(BList* filters)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BList* BLooper::CommonFilterList() const
{
	// TODO: implement
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
	// TODO: implement
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
	// TODO: implement
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

	//	Lock the looper list.  We don't want the looper getting removed from
	//	the list while we're doing list operations.  Also ensures that the
	//	looper doesn't get deleted here (since ~BLooper() has to lock the list
	//	as well to remove itself).
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
			right out from under us.
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
	fRunCalled = false;
	fQueue = new BMessageQueue();
	fCommonFilters = new BList(0);

	if (sTeamID == -1)
	{
		thread_info info;
		get_thread_info(fTaskID,&info);
		sTeamID = info.team;
	}

	BAutoLock ListLock(sLooperListLock);
	AddLooper(this);
}
//------------------------------------------------------------------------------
void BLooper::InitData(const char* name, int32 priority, int32 port_capacity)
{
	fLockSem = create_sem(1, name);

	// Seems kinda strange to me to spawn the thread here and run it elsewhere,
	// but who am I to complain
	fTaskID = spawn_thread(_task0_, name, priority, this);

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
}
//------------------------------------------------------------------------------
void BLooper::_AddMessagePriv(BMessage* msg)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
status_t BLooper::_task0_(void* arg)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void* BLooper::ReadRawFromPort(int32* code, bigtime_t tout)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BMessage* BLooper::ReadMessageFromPort(bigtime_t tout)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
BMessage* BLooper::ConvertToMessage(void* raw, int32 code)
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::task_looper()
{
	// TODO: implement
}
//------------------------------------------------------------------------------
void BLooper::do_quit_requested(BMessage* msg)
{
	// TODO: implement
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

