//
//  $Id$
//
//  This file contains the implementation of the BMessageQueue class
//  for the OpenBeOS project.
//


#include "MessageQueue.h"
#include "Autolock.h"


#ifdef OPENBEOS

// We need to make our BMessageQueue a friend of BMessage.  Since we are
// building BMessageQueue in the OpenBeOS namespace, we need to add this
// ourselves.  The following is a hack to enable this friendship.  It is
// just the Be definition of BMessage with an extra friend class for
// OpenBeOS::BMessageQueue.  This should be removed once OpenBeOS has its
// own BMessage implemented.

// Copy from Message.h starts here

#include <OS.h>
#include <Rect.h>
#include <DataIO.h>
#include <Flattenable.h>

#include <AppDefs.h>		/* For convenience */
#include <TypeConstants.h>	/* For convenience */

class	BBlockCache;
class	BMessenger;
class	BHandler;
class	BString;

/*-----------------------------------------*/
/*----- Private or reserved ---------------*/
extern "C" void		_msg_cache_cleanup_();
extern "C" int		_init_message_();
extern "C" int		_delete_message_();
/*-----------------------------------------*/


/*-------------------------------------------------------------*/
/* ------- Name lengths and Scripting specifiers ------------- */

#define B_FIELD_NAME_LENGTH			255
#define B_PROPERTY_NAME_LENGTH		255

enum {
	B_NO_SPECIFIER = 0,
	B_DIRECT_SPECIFIER = 1,
	B_INDEX_SPECIFIER,
	B_REVERSE_INDEX_SPECIFIER,
	B_RANGE_SPECIFIER,
	B_REVERSE_RANGE_SPECIFIER,
	B_NAME_SPECIFIER,
	B_ID_SPECIFIER,

	B_SPECIFIERS_END = 128
	/* app-defined specifiers start at B_SPECIFIERS_END+1 */
};


/*-------------------------------------------------------------*/
/* --------- BMessage class----------------------------------- */

class BMessage {

public:
		uint32		what;

					BMessage();
					BMessage(uint32 what);
					BMessage(const BMessage &a_message);
virtual				~BMessage();

		BMessage	&operator=(const BMessage &msg);

/* Statistics and misc info */
		status_t	GetInfo(type_code typeRequested, int32 which, char **name,
							type_code *typeReturned, int32 *count = NULL) const;

		status_t	GetInfo(const char *name, type_code *type, int32 *c = 0) const;
		status_t	GetInfo(const char *name, type_code *type, bool *fixed_size) const;

		int32		CountNames(type_code type) const;
		bool		IsEmpty() const;
		bool		IsSystem() const;
		bool		IsReply() const;
		void		PrintToStream() const;

		status_t	Rename(const char *old_entry, const char *new_entry);

/* Delivery info */
		bool		WasDelivered() const;
		bool		IsSourceWaiting() const;
		bool		IsSourceRemote() const;
		BMessenger	ReturnAddress() const;
		const BMessage	*Previous() const;
		bool		WasDropped() const;
		BPoint		DropPoint(BPoint *offset = NULL) const;

/* Replying */
		status_t	SendReply(uint32 command, BHandler *reply_to = NULL);
		status_t	SendReply(BMessage *the_reply, BHandler *reply_to = NULL,
							bigtime_t timeout = B_INFINITE_TIMEOUT);
		status_t	SendReply(BMessage *the_reply, BMessenger reply_to,
							bigtime_t timeout = B_INFINITE_TIMEOUT);
	
		status_t	SendReply(uint32 command, BMessage *reply_to_reply);
		status_t	SendReply(BMessage *the_reply, BMessage *reply_to_reply,
							bigtime_t send_timeout = B_INFINITE_TIMEOUT,
							bigtime_t reply_timeout = B_INFINITE_TIMEOUT);

/* Flattening data */
		ssize_t		FlattenedSize() const;
		status_t	Flatten(char *buffer, ssize_t size) const;
		status_t	Flatten(BDataIO *stream, ssize_t *size = NULL) const;
		status_t	Unflatten(const char *flat_buffer);
		status_t	Unflatten(BDataIO *stream);


/* Specifiers (scripting) */
		status_t	AddSpecifier(const char *property);
		status_t	AddSpecifier(const char *property, int32 index);
		status_t	AddSpecifier(const char *property, int32 index, int32 range);
		status_t	AddSpecifier(const char *property, const char *name);
		status_t	AddSpecifier(const BMessage *specifier);

		status_t	SetCurrentSpecifier(int32 index);
		status_t	GetCurrentSpecifier(int32 *index, BMessage *specifier = NULL,
							int32 *form = NULL, const char **property = NULL) const;
		bool		HasSpecifiers() const;
		status_t	PopSpecifier();

/* Adding data */
		status_t	AddRect(const char *name, BRect a_rect);
		status_t	AddPoint(const char *name, BPoint a_point);
		status_t	AddString(const char *name, const char *a_string);
		status_t	AddString(const char *name, const BString& a_string);
		status_t	AddInt8(const char *name, int8 val);
		status_t	AddInt16(const char *name, int16 val);
		status_t	AddInt32(const char *name, int32 val);
		status_t	AddInt64(const char *name, int64 val);
		status_t	AddBool(const char *name, bool a_boolean);
		status_t	AddFloat(const char *name, float a_float);
		status_t	AddDouble(const char *name, double a_double);
		status_t	AddPointer(const char *name, const void *ptr);
		status_t	AddMessenger(const char *name, BMessenger messenger);
		status_t	AddRef(const char *name, const entry_ref *ref);
		status_t	AddMessage(const char *name, const BMessage *msg);
		status_t	AddFlat(const char *name, BFlattenable *obj, int32 count = 1);
		status_t	AddData(const char *name, type_code type, const void *data,
						ssize_t numBytes, bool is_fixed_size = true, int32 count = 1);

/* Removing data */
		status_t	RemoveData(const char *name, int32 index = 0);
		status_t	RemoveName(const char *name);
		status_t	MakeEmpty();

/* Finding data */
		status_t	FindRect(const char *name, BRect *rect) const;
		status_t	FindRect(const char *name, int32 index, BRect *rect) const;
		status_t	FindPoint(const char *name, BPoint *pt) const;
		status_t	FindPoint(const char *name, int32 index, BPoint *pt) const;
		status_t	FindString(const char *name, const char **str) const;
		status_t	FindString(const char *name, int32 index, const char **str) const;
		status_t	FindString(const char *name, BString *str) const;
		status_t	FindString(const char *name, int32 index, BString *str) const;
		status_t	FindInt8(const char *name, int8 *value) const;
		status_t	FindInt8(const char *name, int32 index, int8 *val) const;
		status_t	FindInt16(const char *name, int16 *value) const;
		status_t	FindInt16(const char *name, int32 index, int16 *val) const;
		status_t	FindInt32(const char *name, int32 *value) const;
		status_t	FindInt32(const char *name, int32 index, int32 *val) const;
		status_t	FindInt64(const char *name, int64 *value) const;
		status_t	FindInt64(const char *name, int32 index, int64 *val) const;
		status_t	FindBool(const char *name, bool *value) const;
		status_t	FindBool(const char *name, int32 index, bool *value) const;
		status_t	FindFloat(const char *name, float *f) const;
		status_t	FindFloat(const char *name, int32 index, float *f) const;
		status_t	FindDouble(const char *name, double *d) const;
		status_t	FindDouble(const char *name, int32 index, double *d) const;
		status_t	FindPointer(const char *name, void **ptr) const;
		status_t	FindPointer(const char *name, int32 index,  void **ptr) const;
		status_t	FindMessenger(const char *name, BMessenger *m) const;
		status_t	FindMessenger(const char *name, int32 index, BMessenger *m) const;
		status_t	FindRef(const char *name, entry_ref *ref) const;
		status_t	FindRef(const char *name, int32 index, entry_ref *ref) const;
		status_t	FindMessage(const char *name, BMessage *msg) const;
		status_t	FindMessage(const char *name, int32 index, BMessage *msg) const;
		status_t	FindFlat(const char *name, BFlattenable *obj) const;
		status_t	FindFlat(const char *name, int32 index, BFlattenable *obj) const;
		status_t	FindData(const char *name, type_code type,
							const void **data, ssize_t *numBytes) const;
		status_t	FindData(const char *name, type_code type, int32 index,
							const void **data, ssize_t *numBytes) const;

/* Replacing data */
		status_t	ReplaceRect(const char *name, BRect a_rect);
		status_t	ReplaceRect(const char *name, int32 index, BRect a_rect);
		status_t	ReplacePoint(const char *name, BPoint a_point);
		status_t	ReplacePoint(const char *name, int32 index, BPoint a_point);
		status_t	ReplaceString(const char *name, const char *string);
		status_t	ReplaceString(const char *name, int32 index, const char *string);
		status_t	ReplaceString(const char *name, const BString& string);
		status_t	ReplaceString(const char *name, int32 index, const BString& string);
		status_t	ReplaceInt8(const char *name, int8 val);
		status_t	ReplaceInt8(const char *name, int32 index, int8 val);
		status_t	ReplaceInt16(const char *name, int16 val);
		status_t	ReplaceInt16(const char *name, int32 index, int16 val);
		status_t	ReplaceInt32(const char *name, int32 val);
		status_t	ReplaceInt32(const char *name, int32 index, int32 val);
		status_t	ReplaceInt64(const char *name, int64 val);
		status_t	ReplaceInt64(const char *name, int32 index, int64 val);
		status_t	ReplaceBool(const char *name, bool a_bool);
		status_t	ReplaceBool(const char *name, int32 index, bool a_bool);
		status_t	ReplaceFloat(const char *name, float a_float);
		status_t	ReplaceFloat(const char *name, int32 index, float a_float);
		status_t	ReplaceDouble(const char *name, double a_double);
		status_t	ReplaceDouble(const char *name, int32 index, double a_double);
		status_t	ReplacePointer(const char *name, const void *ptr);
		status_t	ReplacePointer(const char *name,int32 index,const void *ptr);
		status_t	ReplaceMessenger(const char *name, BMessenger messenger);
		status_t	ReplaceMessenger(const char *name, int32 index, BMessenger msngr);
		status_t	ReplaceRef(	const char *name,const entry_ref *ref);
		status_t	ReplaceRef(	const char *name, int32 index, const entry_ref *ref);
		status_t	ReplaceMessage(const char *name, const BMessage *msg);
		status_t	ReplaceMessage(const char *name, int32 index, const BMessage *msg);
		status_t	ReplaceFlat(const char *name, BFlattenable *obj);
		status_t	ReplaceFlat(const char *name, int32 index, BFlattenable *obj);
		status_t	ReplaceData(const char *name, type_code type,
								const void *data, ssize_t data_size);
		status_t	ReplaceData(const char *name, type_code type, int32 index,
								const void *data, ssize_t data_size);

		void		*operator new(size_t size);
		void		operator delete(void *ptr, size_t size);

/*----- Private, reserved, or obsolete ------------------------------*/
		bool		HasRect(const char *, int32 n = 0) const;
		bool		HasPoint(const char *, int32 n = 0) const;
		bool		HasString(const char *, int32 n = 0) const;
		bool		HasInt8(const char *, int32 n = 0) const;
		bool		HasInt16(const char *, int32 n = 0) const;
		bool		HasInt32(const char *, int32 n = 0) const;
		bool		HasInt64(const char *, int32 n = 0) const;
		bool		HasBool(const char *, int32 n = 0) const;
		bool		HasFloat(const char *, int32 n = 0) const;
		bool		HasDouble(const char *, int32 n = 0) const;
		bool		HasPointer(const char *, int32 n = 0) const;
		bool		HasMessenger(const char *, int32 n = 0) const;
		bool		HasRef(const char *, int32 n = 0) const;
		bool		HasMessage(const char *, int32 n = 0) const;
		bool		HasFlat(const char *, const BFlattenable *) const;
		bool		HasFlat(const char *,int32 ,const BFlattenable *) const;
		bool		HasData(const char *, type_code , int32 n = 0) const;
		BRect		FindRect(const char *, int32 n = 0) const;
		BPoint		FindPoint(const char *, int32 n = 0) const;
		const char	*FindString(const char *, int32 n = 0) const;
		int8		FindInt8(const char *, int32 n = 0) const;
		int16		FindInt16(const char *, int32 n = 0) const;
		int32		FindInt32(const char *, int32 n = 0) const;
		int64		FindInt64(const char *, int32 n = 0) const;
		bool		FindBool(const char *, int32 n = 0) const;
		float		FindFloat(const char *, int32 n = 0) const;
		double		FindDouble(const char *, int32 n = 0) const;

private:

friend class	BMessageQueue;
friend class	OpenBeOS::BMessageQueue;
friend class	BMessenger;
friend class	BApplication;

friend			void		_msg_cache_cleanup_();
friend			BMessage 	*_reconstruct_msg_(uint32,uint32,uint32);
friend inline	void		_set_message_target_(BMessage *, int32, bool);
friend inline	void		_set_message_reply_(BMessage *, BMessenger);
friend inline	int32		_get_message_target_(BMessage *);
friend inline	bool		_use_preferred_target_(BMessage *);

					/* deprecated */
					BMessage(BMessage *a_message);
					
virtual	void		_ReservedMessage1();
virtual	void		_ReservedMessage2();
virtual	void		_ReservedMessage3();

		void		init_data();
		int32		flatten_hdr(uchar *result,
								ssize_t size,
								uchar flags) const;
		status_t	real_flatten(char *result,
								ssize_t size,
								uchar flags) const;
		status_t	real_flatten(BDataIO *stream,
								ssize_t size,
								uchar flags) const;
		char		*stack_flatten(char *stack_ptr,
									ssize_t stack_size,
									bool incl_reply,
									ssize_t *size = NULL) const;
		ssize_t		calc_size(uchar flags) const;
		ssize_t		calc_hdr_size(uchar flags) const;
		ssize_t		min_hdr_size() const;
		status_t	nfind_data(	const char *name,
								type_code type,
								int32 index,
								const void **data,
								ssize_t *data_size) const;
		status_t	copy_data(	const char *name,
								type_code type,
								int32 index,
								void *data,
								ssize_t data_size) const;

		status_t	_send_(port_id port,
							int32 token,
							bool preferred,
							bigtime_t timeout,
							bool reply_required,
							BMessenger &reply_to) const;
		status_t	send_message(port_id port,
								team_id port_owner,
								int32 token,
								bool preferred,
								BMessage *reply,
								bigtime_t send_timeout,
								bigtime_t reply_timeout) const;

		enum		{ sNumReplyPorts = 3 };
static	port_id		sReplyPorts[sNumReplyPorts];
static	long		sReplyPortInUse[sNumReplyPorts];
static	int32		sGetCachedReplyPort();

friend	int _init_message_();
friend	int _delete_message_();
static	BBlockCache	*sMsgCache;

		struct dyn_array {
			int32		fLogicalBytes;
			int32		fPhysicalBytes;
			int32		fChunkSize;		
			int32		fCount;
			int32		fEntryHdrSize;	
		};

		struct entry_hdr  : public dyn_array {
			entry_hdr	*fNext;
			uint32		fType;
			uchar		fNameLength;	
			char		fName[1];
		};

		struct var_chunk {
			int32	fDataSize;				
			char	fData[1];
		};

		void		*da_create(int32 header_size, int32 chunk_size,
								bool fixed, int32 nchunks);
		status_t	da_add_data(dyn_array **da, const void *data, int32 size);
		void		*da_find_data(dyn_array *da, int32 index,
									int32 *size = NULL) const;
		status_t	da_delete_data(dyn_array **pda, int32 index);
		status_t	da_replace_data(dyn_array **pda, int32 index,
									const void *data, int32 dsize);
		int32		da_calc_size(int32 hdr_size, int32 chunksize,
								bool is_fixed, int32 nchunks) const;
		void		*da_grow(dyn_array **pda, int32 increase);
		void		da_dump(dyn_array *da);

		int32		da_chunk_hdr_size() const
						{ return sizeof(int32); }
		int32		da_chunk_size(var_chunk *v) const
						{ return (v->fDataSize + da_chunk_hdr_size() + 7) & ~7; }
		var_chunk	*da_first_chunk(dyn_array *da) const
						{ return (var_chunk *) da_start_of_data(da); }
		var_chunk	*da_next_chunk(var_chunk *v) const
						{ return (var_chunk *) (((char*) v) + da_chunk_size(v)); }
		var_chunk	*da_chunk_ptr(void *data) const
						{ return (var_chunk*) (((char *) data) - da_chunk_hdr_size()); }

		int32		da_pad_8(int32 val) const
						{ return (val + 7) & ~7; }
		int32		da_total_size(dyn_array *da) const
						{ return (int32)sizeof(dyn_array) + da->fEntryHdrSize +
											da->fPhysicalBytes; }
		char		*da_start_of_data(dyn_array *da) const
						{ return ((char *) da) + (sizeof(dyn_array) +
											da->fEntryHdrSize); }
		bool		da_is_mini_data(dyn_array *da) const
						{ return ((da->fLogicalBytes <= (int32) UCHAR_MAX) &&
											(da->fCount <= (int32) UCHAR_MAX));}
		void		da_swap_var_sized(dyn_array *da);
		void		da_swap_fixed_sized(dyn_array *da);

		BMessage			*link;
		int32				fTarget;	
		BMessage			*fOriginal;
		uint32				fChangeCount;
		int32				fCurSpecifier;
		uint32				fPtrOffset;
		uint32				_reserved[3];

		BMessage::entry_hdr	*fEntries;

		struct {
			port_id				port;
			int32				target;
			team_id				team;
			bool				preferred;
		} fReplyTo;

		bool				fPreferred;
		bool				fReplyRequired;
		bool				fReplyDone;
		bool				fIsReply;
		bool				fWasDelivered;
		bool				fReadOnly;
		bool				fHasSpecifiers;	
};

// Copy from Message.h ends here.

namespace OpenBeOS {
#endif


/*
 *  Method: BMessageQueue::BMessageQueue()
 *   Descr: This method is the only constructor for a BMessageQueue.  Once the
 *          constructor completes, the BMessageQueue is created with no BMessages
 *          in it.
 *
 */
BMessageQueue::BMessageQueue() :
	fTheQueue(NULL), fQueueTail(NULL), fMessageCount(0)
{
}


/*
 *  Method: BMessageQueue::~BMessageQueue()
 *   Descr: This is the desctructor for the BMessageQueue.  It iterates over
 *          any messages left on the queue and deletes them.
 *
 *		    The implementation is careful not to release the lock when the
 *          BMessageQueue is deconstructed.  If the lock is released, it is
 *          possible another thread will start an AddMessage() operation before
 *          the BLocker is deleted.  The safe thing to do is not to unlock the
 *          BLocker from the destructor once it is acquired. That way, any thread
 *          waiting to do a AddMessage() will fail to acquire the lock since the
 *          BLocker will be deleted before they can acquire it.
 *
 */
BMessageQueue::~BMessageQueue()
{
	if (fLocker.Lock()) {
		BMessage *theMessage = fTheQueue;
		while (theMessage != NULL) {
			BMessage *messageToDelete = theMessage;
			theMessage = theMessage->link;
			delete messageToDelete;
		}
	}
}


/*
 *  Method: BMessageQueue::AddMessage()
 *   Descr: This method adds a BMessage to the queue.  It makes a couple of
 *          assumptions:
 *             - The BMessage was allocated on the heap with new.  Since the
 *               destructor delete's BMessages left on the queue, this must be
 *               true.  The same assumption is made with Be's implementation.
 *             - The BMessage is not already on this or any other BMessageQueue.
 *               If it is, the queue it is already on will be corrupted.  Be's
 *               implementation makes this assumption also and does corrupt
 *               BMessageQueues where this is violated.
 *
 */
void
BMessageQueue::AddMessage(BMessage *message)
{
	if (message == NULL) {
		return;
	}
	
	BAutolock theAutoLocker(fLocker);
	
	if (theAutoLocker.IsLocked()) {
		
		// The message passed in will be the last message on the queue so its
		// link member should be set to null.
		message->link = NULL;
		
		// We now have one more BMessage on the queue.
		fMessageCount++;
		
		// If there are no BMessages on the queue.
		if (fQueueTail == NULL) {
			// Then this message is both the start and the end of the queue.
			fTheQueue = message;
			fQueueTail = message;
		} else {
			// If there are already messages on the queue, then the put this
			// BMessage at the end.  The last BMessage prior to this AddMessage()
			// is fQueueTail.  The BMessage at fQueueTail needs to point to the
			// new last message, the one being added.
			fQueueTail->link = message;
			
			// Now update the fQueueTail to point to this new last message.
			fQueueTail = message;
		}
	}
}


/*
 *  Method: BMessageQueue::RemoveMessage()
 *   Descr: This method searches the queue for a particular BMessage.  If
 *          it is found, it is removed from the queue.
 * 
 */
void
BMessageQueue::RemoveMessage(BMessage *message)
{
	if (message == NULL) {
		return;
	}
	
	BAutolock theAutoLocker(fLocker);
	
	if (theAutoLocker.IsLocked()) {
		
		// If the message to be removed is at the front of the queue.
		if (fTheQueue == message) {
			// We need to special case the handling of removing the first element.
			// First, the new front element will be the next one.
			fTheQueue = fTheQueue->link;
			
			// Must decrement the count of elements since the front one is being
			// removed.
			fMessageCount--;
			
			// If the new front element is NULL, then that means that the queue
			// is now empty.  That means that fQueueTail must be set to NULL.
			if (fTheQueue == NULL) {
				fQueueTail = NULL;
			}
			
			// We have found the message and removed it in this case.  We can
			// bail out now.  The autolocker will take care of releasing the
			// lock for us.
			return;
		}
		
		// The message to remove is not the first one, so we need to scan the
		// queue.  Get a message iterator and set it to the first element.
		BMessage *messageIter = fTheQueue;
		
		// While we are not at the end of the list.
		while (messageIter != NULL) {
			// If the next message after this (ie second, then third etc) is 
			// the one we are looking for.
			if (messageIter->link == message) {
				// At this point, this is what we have:
				//    messageIter - the BMessage in the queue just before the
				//                  match
				//    messageIter->link - the BMessage which matches message
				//    message - the same as messageIter->link
				//    message->link - the element after the match
				//
				// The next step is to link the BMessage just before the match
				// to the one just after the match.  This removes the match from
				// the queue.
				messageIter->link = message->link;
				
				// One less element on the queue.
				fMessageCount--;
				
				// If there is no BMessage after the match is the
				if (message->link == NULL) {
					// That means that we just removed the last element from the
					// queue.  The new last element then must be messageIter.
					fQueueTail = messageIter;
				}
				
				// We can return now because we have a match and removed it.
				return;
			}
			
			// No match yet, go to the next element in the list.
			messageIter = messageIter->link;
		}
	}
}


/*
 *  Method: BMessageQueue::CountMessages()
 *   Descr: This method just returns the number of BMessages on the queue.
 */
int32
BMessageQueue::CountMessages(void) const
{
    return fMessageCount;
}


/*
 *  Method: BMessageQueue::IsEmpty()
 *   Descr: This method just returns true if there are no BMessages on the queue.
 */
bool
BMessageQueue::IsEmpty(void) const
{
    return (fMessageCount == 0);
}


/*
 *  Method: BMessageQueue::FindMessage()
 *   Descr: This method searches the queue for the index'th BMessage.  The first
 *          BMessage is at index 0, the second at index 1 etc.  The BMessage
 *          is returned if it is found.  If no BMessage exists at that index
 *          (ie the queue is not that long or the index is invalid) NULL is
 *          returned.
 *
 *          This method does not lock the BMessageQueue so there is risk that
 *          the queue could change in the course of the search.  Be's
 *          implementation must do the same, unless they do some funky casting.
 *          The method is declared const which means it cannot modify the data
 *          members.  Because it cannot modify the data members, it cannot
 *          acquire a lock.  So unless they are casting away the const-ness
 *          of the this pointer, this member in Be's implementation does no
 *          locking either.
 */
BMessage *
BMessageQueue::FindMessage(int32 index) const
{
	// If the index is negative or larger than the number of messages on the
	// queue.
	if ((index < 0) || (index >= fMessageCount)) {
		// No match is possible, bail out now.
		return NULL;
	}
	
	// Get a message iterator and initialize it to the start of the queue.
	BMessage *messageIter = fTheQueue;
	
	// While this is not the end of the queue.
	while (messageIter != NULL) {
		// If the index reaches zero, then we have found a match.
		if (index == 0) {
			// Because this is a match, break out of the while loop so we can
			// return the message pointed to messageIter.		
			break;
		}
		
		// No match yet, decrement the index.  We will have a match once index
		// reaches zero.
		index--;
		// Increment the messageIter to the next BMessage on the queue.
		messageIter = messageIter->link;
	}
	
	// If no match was found, messageIter will be NULL since that is the only
	// way out of the loop.  If a match was found, the messageIter will point
	// to that match.
    return messageIter;
}


/*
 *  Method: BMessageQueue::FindMessage()
 *   Descr: This method searches the queue for the index'th BMessage that has a
 *          particular what code.  The first BMessage with that what value is at
 *          index 0, the second at index 1 etc.  The BMessage is returned if it
 *          is found.  If no matching BMessage exists at that index NULL is
 *          returned.
 *
 *          This method does not lock the BMessageQueue so there is risk that
 *          the queue could change in the course of the search.  Be's
 *          implementation must do the same, unless they do some funky casting.
 *          The method is declared const which means it cannot modify the data
 *          members.  Because it cannot modify the data members, it cannot
 *          acquire a lock.  So unless they are casting away the const-ness
 *          of the this pointer, this member in Be's implementation does no
 *          locking either.
 */
BMessage *
BMessageQueue::FindMessage(uint32 what,
                           int32 index) const
{
	// If the index is negative or larger than the number of messages on the
	// queue.
	if ((index < 0) || (index >= fMessageCount)) {
		// No match is possible, bail out now.
		return NULL;
	}
	
	// Get a message iterator and initialize it to the start of the queue.
	BMessage *messageIter = fTheQueue;
	
	// While this is not the end of the queue.
	while (messageIter != NULL) {
		// If the messageIter points to a BMessage with the what code we are
		// looking for.
		if (messageIter->what == what) {
			// If the index reaches zero, then we have found a match.
			if (index == 0) {
				// Because this is a match, break out of the while loop so we can
				// return the message pointed to messageIter.	
				break;
			}
			// No match yet, decrement the index.  We will have a match once index
			// reaches zero.
			index--;
		}
		// Increment the messageIter to the next BMessage on the queue.
		messageIter = messageIter->link;
	}
	
	// If no match was found, messageIter will be NULL since that is the only
	// way out of the loop.  If a match was found, the messageIter will point
	// to that match.
    return messageIter;
}


/*
 *  Method: BMessageQueue::Lock()
 *   Descr: This member just locks the BMessageQueue so no other thread can acquire
 *          the lock nor make changes to the queue through members like
 *          AddMessage(), RemoveMessage(), NextMessage() or ~BMessageQueue().
 */
bool
BMessageQueue::Lock(void)
{
    return fLocker.Lock();
}


/*
 *  Method: BMessageQueue::Unlock()
 *   Descr: This member releases the lock which was acquired by Lock().
 */
void
BMessageQueue::Unlock(void)
{
	fLocker.Unlock();
}


/*
 *  Method: BMessageQueue::NextMessage()
 *   Descr: This member removes the first BMessage on the queue and returns
 *          it to the caller.  If the queue is empty, NULL is returned.
 */
BMessage *
BMessageQueue::NextMessage(void)
{
	// By default, we will assume that no BMessage is on the queue.
	BMessage *result = NULL;
	BAutolock theAutoLocker(fLocker);
	
	if (theAutoLocker.IsLocked()) {
		// Store the first BMessage in the queue in result.
		result = fTheQueue;
		
		// If the queue is not empty.
		if (fTheQueue != NULL) {
			// Decrement the message count since we are removing an element.
			fMessageCount--;
			// The new front of the list is moved forward thereby removing the
			// first element from the queue.
			fTheQueue = fTheQueue->link;
			// If the queue is empty after removing the front element.
			if (fTheQueue == NULL) {
				// We need to set the tail of the queue to NULL since the queue
				// is now empty.
				fQueueTail = NULL;
			}
		}
	}
    return result;
}


void 
BMessageQueue::_ReservedMessageQueue1(void)
{
}


void 
BMessageQueue::_ReservedMessageQueue2(void)
{
}


void 
BMessageQueue::_ReservedMessageQueue3(void)
{
}

#ifdef OPENBEOS
}
#endif