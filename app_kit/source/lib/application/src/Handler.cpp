//------------------------------------------------------------------------------
//	Handler.cpp
//
/**
	@note	Some musings on the member variable 'fToken'.  In searching a dump
			of libbe.so, I found a struct/class called 'TokenSpace', which has
			various member functions like 'new_token'.  More intriguing is a
			dump of Dano's version of libbe.so:  there is BPrivate::BTokenSpace,
			which also has functions like 'NewToken'.  There's more: one
			version of BTokenSpace::NewToken takes a pointer to another new
			class, BPrivate::BDirectMessageTarget.  Only a constructor,
			destructor and vtable are listed for it, so I'm guessing it's an
			abstract base class (or not very useful ;).  My guess is that
			BDirectMessageTarget facilitates sending messages straight to an
			associated BHandler.  Maybe from app_server?  Probably not, since
			you'd have to do IPC anyway.  But maybe within the same team, or
			even between teams?  It might save unnecessary trips to and from
			app_server for messages which otherwise are entirely client-side.
			
			Back to tokens. There are also these functions in R5:
				_safe_get_server_token_(const BLooper*, int32*)
				BWindow::find_token_and_handler(BMessage*, int32*, BHandler**)
				BWindow::get_server_token

			and Dano adds a few more provocative sounding functions:
				get_handler_token(short, void*) (listed 3 times, actually)
				new_handler_token(short, void*)
				remove_handler_token(short, void*)

			Taken all together, I think there's a sound argument to be made that
			each BHandler has an int32 token associated with it in app_server.

			Furthermore, there is, in R5, a function set_token_type(long, short)
			which leads me to think that although BHandler's have server tokens
			associated with them, the tokening facility is, in fact, generic.
			These functions would seem to support that theory:
				BBitmap::get_server_token
				BPicture::set_token

			An important question is whether tokens are generated on the client
			side and registered with the server or generated on the server and
			given back to the client.  The exported functions of TokenSpace in
			libbe's dump are:
				~TokenSpace
				TokenSpace
				adjust_free(long, long)
				dump()
				find_free_entry(long*, long*)
				full_search_adjust()
				get_token(void**)
				get_token(short*, void**)
				new_token(long, short, void*)
				new_token_array(long)
				remove_token(long)
				set_token_type(long, short)

			TokenSpace functions are also exported from app_server:
				~TokenSpace
				TokenSpace
				adjust_free(long, long)
				cleanup_dead(long)
				delete_atom(SAtom*)
				dump_tokens()
				find_free_entry(long*, long*)
				full_search_adjust()
				get_token(long, void**)
				get_token(long, short*, void**)
				get_token_by_type(int*, short, long, void**)
				grab_atom(long, SAtom**)
				iterate_tokens(long, short,
							   unsigned long(*)(long, long, short, void*, void*),
							   void*)
				new_token(long, short, void*)
				new_token_array(long)
				remove_token(long)
				set_token_type(long, short)

			While there are common functions in both locations, the fact that
			app_server exports TokenSpace functions which libbe.so does not
			leads me to believe that libbe.so and app_server each have their own
			versions of TokenSpace.  While it's possible that the libbe version
			simply acts as a proxy for the app_server version, it seems not
			only inefficient but unnecessary as well:  client-side objects
			exists in a different "namespace" than server-side objects, so over-
			lap between the two token sets shouldn't be an issue.

			Obviously, I can't be entirely sure this is how R5 does it, but it
			seems like a reasonable design to follow for our purposes.
 */
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>

// System Includes -------------------------------------------------------------
#include <be/app/AppDefs.h>
#include <be/app/Handler.h>
#include <be/app/Looper.h>
#include <be/app/Message.h>
#include <be/app/MessageFilter.h>
#include <be/app/Messenger.h>
#include <be/app/PropertyInfo.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------
#define USE_BOBJECTLIST	1

// Globals ---------------------------------------------------------------------
using std::map;
using std::vector;

const char*	gArchiveNameField = "_name";
static property_info gHandlerPropInfo[] =
{
	{
		// name
		"Suites",
		// commands
		{B_GET_PROPERTY},
		// specifiers
		{B_DIRECT_SPECIFIER},
		// usage
		NULL,
		// extra data
		0,
		// types
		{ },
		// ctypes (compound_type)
		{
			// ctypes[0]
			{
				// pairs[0]
				{
					{
						// name
						"suites",
						// type
						B_STRING_TYPE
					} 
				}
			},
			// ctypes[1]
			{
				// pairs[1]
				{
					{
						// name
						"messages",
						// type
						B_PROPERTY_INFO_TYPE
					}
				}
			}
		},
		// reserved
		{}
	},
	{
		"Messenger",
			{B_GET_PROPERTY},
			{B_DIRECT_SPECIFIER},
			NULL, 0,
			{ B_MESSENGER_TYPE },
			{},
			{}
	},
	{
		"InternalName",
			{B_GET_PROPERTY},
			{B_DIRECT_SPECIFIER},
			NULL, 0,
			{ B_STRING_TYPE },
			{},
			{}
	},
	{}
};

bool FilterDeleter(void* filter);
bool FilterLooperSetter(void* filter, void* looper);

typedef map<unsigned long, vector<BHandler*> >	THandlerObserverMap;
typedef map<unsigned long, vector<BMessenger> >	TMessengerObserverMap;
//------------------------------------------------------------------------------
// TODO: Change to BPrivate::BObserverList if possible
class _ObserverList
{
	public:
		_ObserverList(void);
		~_ObserverList(void);
		status_t SendNotices(unsigned long, BMessage const *);
		status_t StartObserving(BHandler *, unsigned long);
		status_t StartObserving(const BMessenger&, unsigned long);
		status_t StopObserving(BHandler *, unsigned long);
		status_t StopObserving(const BMessenger&, unsigned long);
		bool IsEmpty();

	private:
		THandlerObserverMap		fHandlerMap;
		TMessengerObserverMap	fMessengerMap;
};
//------------------------------------------------------------------------------
class MatchWhat
{
public:
	int Compare(_ObserverWhatList const *, _ObserverWhatList const *);

	// not sure what this one returns
	void operator()(_ObserverWhatList const *) const;
};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
BHandler::BHandler(const char* name)
	:	BArchivable()
{
	InitData(name);
}
//------------------------------------------------------------------------------
BHandler::~BHandler()
{
	if (fName)
	{
		free(fName);
	}
}
//------------------------------------------------------------------------------
BHandler::BHandler(BMessage* data)
	:	BArchivable(data)
{
	const char* name = NULL;
	data->FindString(gArchiveNameField, &name);
	InitData(name);
}
//------------------------------------------------------------------------------
BArchivable* BHandler::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "BHandler"))
	{
		return NULL;
	}

	return new BHandler(data);
}
//------------------------------------------------------------------------------
status_t BHandler::Archive(BMessage* data, bool deep) const
{
	status_t err = BArchivable::Archive(data, deep);
	if (!err)
	{
		err = data->AddString(gArchiveNameField, fName);
	}

	return err;
}
//------------------------------------------------------------------------------
void BHandler::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		case B_GET_PROPERTY:
		{
			BMessage	Specifier;
			int32		form;
			const char*	prop;
			int32		cur;
			status_t	err;

			err = message->GetCurrentSpecifier(&cur, &Specifier, &form, &prop);
			if (!err)
			{
				BMessage Reply(B_REPLY);
				if (strcmp(prop, "Suites") == 0)
				{
					if (GetSupportedSuites(&Reply) == B_OK &&
						Reply.AddInt32("error", B_OK) == B_OK)
					{
						message->SendReply(&Reply);
					}
				}
				else if (strcmp(prop, "Messenger") == 0)
				{
					if (Reply.AddMessenger("result", this) == B_OK &&
						Reply.AddInt32("error", B_OK) == B_OK)

					{
						message->SendReply(&Reply);
					}
				}
				else if (strcmp(prop, "InternalName") == 0)
				{
					if (Reply.AddString("result", Name()) == B_OK &&
						Reply.AddInt32("error", B_OK) == B_OK)

					{
						message->SendReply(&Reply);
					}
				}
				else
				{
					// Should never be here
					debugger("We are *not* supposed to be here");
				}
			}
			break;
		}

		default:
			if (fNextHandler)
			{
				fNextHandler->MessageReceived(message);
			}
			else
			{
				message->SendReply(B_MESSAGE_NOT_UNDERSTOOD);
			}
			break;
	}
}
//------------------------------------------------------------------------------
BLooper* BHandler::Looper() const
{
	return fLooper;
}
//------------------------------------------------------------------------------
void BHandler::SetName(const char* name)
{
	if (fName)
	{
		free(fName);
		fName = NULL;
	}

	if (name)
	{
		fName = strdup(name);
	}
}
//------------------------------------------------------------------------------
const char* BHandler::Name() const
{
	return fName;
}
//------------------------------------------------------------------------------
void BHandler::SetNextHandler(BHandler* handler)
{
	// NOTE:  I'm sure some sort of threading protection should happen here,
	// hopefully the spec-mandated BLooper lock is sufficient.
	// TODO: implement correctly
	if (fLooper->IsLocked() && fLooper == handler->Looper())
	{
		fNextHandler = handler;
	}
}
//------------------------------------------------------------------------------
BHandler* BHandler::NextHandler() const
{
	return fNextHandler;
}
//------------------------------------------------------------------------------
void BHandler::AddFilter(BMessageFilter* filter)
{
	if (!fLooper)
	{
		// TODO: error handling
		return;
	}

	if (!fLooper->IsLocked())
	{
		// TODO: error handling
		return;
	}

	if (!fFilters)
	{
		fFilters = new BList;
	}

	fFilters->AddItem(filter);
}
//------------------------------------------------------------------------------
bool BHandler::RemoveFilter(BMessageFilter* filter)
{
	if (!fLooper)
	{
		// TODO: error handling
		return false;
	}

	if (!fLooper->IsLocked())
	{
		// TODO: error handling
		return false;
	}

	if (fFilters)
	{
		if (fFilters->RemoveItem((void*)filter))
		{
			filter->SetLooper(NULL);
			return true;
		}
	}

	return false;
}
//------------------------------------------------------------------------------
void BHandler::SetFilterList(BList* filters)
{
	if (!fLooper)
	{
		// TODO: error handling
		return;
	}

	if (!fLooper->IsLocked())
	{
		// TODO: error handling
		return;
	}

	// NOTE: I would like to use BObjectList internally, but this function is
	// spec'd such that fFilters would get deleted and then assigned 'filters',
	// which would obviously mess this up.  Wondering if anyone ever assigns
	// a list of filters and then checks against FilterList() to see if they
	// are the same.
	// TODO: Explore isses with using BObjectList
	if (fFilters)
	{
		fFilters->DoForEach(FilterDeleter);
		delete fFilters;
	}

	fFilters = filters;
	if (fFilters)
	{
		fFilters->DoForEach(FilterLooperSetter, (void*)fLooper);
	}
}
//------------------------------------------------------------------------------
BList* BHandler::FilterList()
{
	return fFilters;
}
//------------------------------------------------------------------------------
bool BHandler::LockLooper()
{
/**
	@note	BeBook says that this function "retrieves the handler's looper and
			unlocks it in a pseudo-atomic operation, thus avoiding a race
			condition."  What "pseudo-atomic" would look completely escapes me,
			so we'll go with the dumb version for now.  Maybe I should use a
			benaphore?
 */
	// TODO: implement correctly
	if (fLooper)
	{
		return fLooper->Lock();
	}

	return false;
}
//------------------------------------------------------------------------------
status_t BHandler::LockLooperWithTimeout(bigtime_t timeout)
{
/**
	@note	BeBook says that this function "retrieves the handler's looper and
			unlocks it in a pseudo-atomic operation, thus avoiding a race
			condition."  What "pseudo-atomic" would look completely escapes me,
			so we'll go with the dumb version for now.  Maybe I should use a
			benaphore?
 */
	// TODO: implement correctly
	if (fLooper)
	{
		return fLooper->LockWithTimeout(timeout);
	}

	return B_BAD_VALUE;
}
//------------------------------------------------------------------------------
void BHandler::UnlockLooper()
{
/**
	@note	BeBook says that this function "retrieves the handler's looper and
			unlocks it in a pseudo-atomic operation, thus avoiding a race
			condition."  What "pseudo-atomic" would look completely escapes me,
			so we'll go with the dumb version for now.  Maybe I should use a
			benaphore?
 */
	// TODO: implement correctly
	if (fLooper)
	{
		fLooper->Unlock();
	}
}
//------------------------------------------------------------------------------
BHandler* BHandler::ResolveSpecifier(BMessage* msg, int32 index,
									 BMessage* specifier, int32 form,
									 const char* property)
{
	// Straight from the BeBook
	BPropertyInfo PropertyInfo(gHandlerPropInfo);
	if (PropertyInfo.FindMatch(msg, index, specifier, form, property) >= 0)
	{
		return this;
	}

	BMessage Reply(B_MESSAGE_NOT_UNDERSTOOD);
	Reply.AddInt32("error", B_BAD_SCRIPT_SYNTAX);
	Reply.AddString("message", "Didn't understand the specifier(s)");
	msg->SendReply(&Reply);

	return NULL;
}
//------------------------------------------------------------------------------
status_t BHandler::GetSupportedSuites(BMessage* data)
{
/**
	@note	This is the output from the original implementation (calling
			PrintToStream() on both data and the contained BPropertyInfo):

BMessage: what =  (0x0, or 0)
    entry         suites, type='CSTR', c=1, size=21, data[0]: "suite/vnd.Be-handler"
    entry       messages, type='SCTD', c=1, size= 0,
      property   commands                       types                specifiers
--------------------------------------------------------------------------------
        Suites   PGET                                               1
                 (RTSC,suites)
                 (DTCS,messages)

     Messenger   PGET                          GNSM                 1
  InternalName   PGET                          RTSC                 1

			With a good deal of trial and error, I determined that the
			parenthetical clauses are entries in the 'ctypes' field of
			property_info.  'ctypes' is an array of 'compound_type', which
			contains an array of 'field_pair's.  I haven't the foggiest what
			either 'compound_type' or 'field_pair' is for, being as the
			scripting docs are so bloody horrible.  The corresponding
			property_info array is declared in the globals section.
 */
	status_t err;
	if (!data)
	{
		err = B_BAD_VALUE;
	}

	if (!err)
	{
		err = data->AddString("Suites", "suite/vnd.Be-handler");
		if (!err)
		{
			BPropertyInfo PropertyInfo(gHandlerPropInfo);
			err = data->AddFlat("message", &PropertyInfo);
		}
	}

	return err;
}
//------------------------------------------------------------------------------
status_t BHandler::StartWatching(BMessenger Messenger, uint32 what)
{
	return fObserverList->StartObserving(&Messenger, what);
}
//------------------------------------------------------------------------------
status_t BHandler::StartWatchingAll(BMessenger Messenger)
{
	return fObserverList->StartObserving(&Messenger, B_OBSERVER_OBSERVE_ALL);
}
//------------------------------------------------------------------------------
status_t BHandler::StopWatching(BMessenger Messenger, uint32 what)
{
	return fObserverList->StopObserving(&Messenger, what);
}
//------------------------------------------------------------------------------
status_t BHandler::StopWatchingAll(BMessenger Messenger)
{
	return fObserverList->StopObserving(&Messenger, B_OBSERVER_OBSERVE_ALL;
}
//------------------------------------------------------------------------------
status_t BHandler::StartWatching(BHandler* Handler, uint32 what)
{
	return fObserverList->StartObserving(Handler, what);
}
//------------------------------------------------------------------------------
status_t BHandler::StartWatchingAll(BHandler* Handler)
{
	return fObserverList->StartObserving(Handler, B_OBSERVER_OBSERVE_ALL;
}
//------------------------------------------------------------------------------
status_t BHandler::StopWatching(BHandler* Handler, uint32 what)
{
	return fObserverList->StopObserving(Handler, what);
}
//------------------------------------------------------------------------------
status_t BHandler::StopWatchingAll(BHandler* Handler)
{
	return fObserverList->StopObserving(Handler, B_OBSERVER_OBSERVE_ALL;
}
//------------------------------------------------------------------------------
status_t BHandler::Perform(perform_code d, void* arg)
{
	return BArchivable::Perform(d, arg);
}
//------------------------------------------------------------------------------
void BHandler::SendNotices(uint32 what, const BMessage* msg)
{
	fObserverList->SendNotices(what, msg);
}
//------------------------------------------------------------------------------
bool BHandler::IsWatched() const
{
	return fObserverList && !fObserverList->IsEmpty();
}
//------------------------------------------------------------------------------
void BHandler::_ReservedHandler2()
{
	// Unused
	;
}
//------------------------------------------------------------------------------
void BHandler::_ReservedHandler3()
{
	// Unused
	;
}
//------------------------------------------------------------------------------
void BHandler::_ReservedHandler4()
{
	// Unused
	;
}
//------------------------------------------------------------------------------
void BHandler::InitData(const char* name)
{
	SetName(name);

	fLooper			= NULL;
	fNextHandler	= NULL;
	fFilters		= NULL;
	fObserverList	= NULL;
}
//------------------------------------------------------------------------------
BHandler::BHandler(const BHandler& )
{
	// No copy construction allowed.
	;
}
//------------------------------------------------------------------------------
BHandler& BHandler::operator=(const BHandler& )
{
	// No assignments allowed.
	return *this;
}
//------------------------------------------------------------------------------
void BHandler::SetLooper(BLooper* loop)
{
	fLooper = loop;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//	#pragma mark -
//	#pragma mark _ObserverList
//	#pramga mark -
//------------------------------------------------------------------------------
_ObserverList::_ObserverList(void)
{
}
//------------------------------------------------------------------------------
_ObserverList::~_ObserverList(void)
{
}
//------------------------------------------------------------------------------
void _ObserverList::SendNotices(unsigned long what, BMessage const* Message)
{
	// Having to new a temporary is really irritating ...
	BMessage* CopyMsg = NULL;
	if (Message)
	{
		CopyMsg = new BMessage(*Message);
		CopyMsg->what = B_OBSERVER_NOTICE_CHANGE;
		CopyMsg->AddInt32(B_OBSERVE_ORIGINAL_WHAT, Message->what);
	}
	else
	{
		CopyMsg = new BMessage(B_OBSERVER_NOTICE_CHANGE);
	}

	CopyMsg->AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	vector<BHandler*>& Handlers = fHandlerMap[what];
	for (uint32 i = 0; i < Handlers.size(); ++i)
	{
		BMessenger msgr(Handlers[i]);
		msgr.SendMessage(CopyMsg);
	}

	vector<BMessenger>& Messengers = fHandlerMap[what];
	for (uint32 i = 0; i < Messengers.size(); ++i)
	{
		Messengers[i].SendMessage(CopyMsg);
	}

	// Gotta make sure to clean up the annoying temporary ...
	delete CopyMsg;
}
//------------------------------------------------------------------------------
status_t _ObserverList::StartObserving(BHandler* Handler, unsigned long what)
{
	if (!Handler)
	{
		return B_BAD_HANDLER;
	}

	vector<BHandler*>& Handlers = fHandlerMap[what];
	vector<BHandler*>::iterator iter;
	iter = find(Handler.begin(), Handlers.end(), Handler);
	if (iter != Handlers.end())
	{
		// TODO: verify
		return B_OK;
	}

	Handlers.push_back(Handler);
	return B_OK;
}
//------------------------------------------------------------------------------
status_t _ObserverList::StartObserving(const BMessenger& Messenger,
									   unsigned long what)
{
	vector<BMessenger>& Messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(Messengers.begin(), Messengers.end(), Messenger);
	if (iter != Messengers.end())
	{
		// TODO: verify
		return B_OK;
	}

	Messengers.push_back(Messenger);
	return B_OK;
}
//------------------------------------------------------------------------------
status_t _ObserverList::StopObserving(BHandler* Handler, unsigned long what)
{
	if (Handler)
	{
		vector<BHandler*>& Handlers = fHandlerMap[what];
		vector<BHandler*>::iterator iter;
		iter = find(Handlers.begin(), Handlers.end(), Handler);
		if (iter != Handlers.end())
		{
			Handlers.erase(iter);
			return B_OK;
		}
	}

	return B_BAD_HANDLER;
}
//------------------------------------------------------------------------------
status_t _ObserverList::StopObserving(const BMessenger& Messenger,
									  unsigned long what)
{
	// ???:	What if you call StartWatching(MyMsngr, aWhat) and then call
	//		StopWatchingAll(MyMsnger)?  Will MyMsnger be removed from the aWhat
	//		watcher list?  For now, we'll assume that they're discreet lists
	//		which do no cross checking; i.e., MyMsnger would *not* be removed in
	//		this scenario.
	vector<BMessenger>& Messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(Messengers.begin(), Messengers.end(), Messenger);
	if (iter != Messengers.end())
	{
		Messengers.erase(iter);
		return B_OK;
	}

	return B_BAD_HANDLER;
}
//------------------------------------------------------------------------------
bool _ObserverList::IsEmpty()
{
	return return fHandlerMap.empty() && fMessengerMap.empty();
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
bool FilterDeleter(void* filter)
{
	BMessageFilter* Filter = static_cast<BMessageFilter*>(filter);
	if (Filter)
	{
		delete Filter;
		Filter = NULL;
	}

	return false;
}
//------------------------------------------------------------------------------
bool FilterLooperSetter(void* filter, void* looper)
{
	BMessageFilter* Filter = static_cast<BMessageFilter*>(filter);
	BLooper* Looper = static_cast<BLooper*>(looper);

	if (Filter && Looper)
	{
		Filter->SetLooper(Looper);
	}
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

