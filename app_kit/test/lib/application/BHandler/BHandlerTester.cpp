//------------------------------------------------------------------------------
//	BHandlerTester.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/Message.h>

#ifdef SYSTEM_TEST
#include <be/app/Handler.h>
#include <be/app/Looper.h>
#include <be/app/MessageFilter.h>
#else
#include "../../../../source/lib/application/headers/Handler.h"
#include <be/app/Looper.h>
#include "../../../../source/lib/application/headers/MessageFilter.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "BHandlerTester.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
	BHandler(const char* name)
	@case			Construct with NULL name
	@param	name	NULL
	@results		BHandler::Name() should return NULL
 */
void TBHandlerTester::BHandler1()
{
	BHandler Handler((const char*)NULL);
	assert(Handler.Name() == NULL);
}
//------------------------------------------------------------------------------
/**
	BHandler(const char* name)
	@case			Construct with valid name
	@param	name	valid string
	@results		BHandler::Name() returns name
 */
void TBHandlerTester::BHandler2()
{
	BHandler Handler("name");
	assert(string("name") == Handler.Name());
}
//------------------------------------------------------------------------------
/**
	BHandler(BMessage* archive)
	@case			archive is valid and has field "_name"
	@param	archive	valid BMessage pointer
	@results		BHandler::Name() returns _name
 */
void TBHandlerTester::BHandler3()
{
	BMessage Archive;
	Archive.AddString("_name", "the name");
	BHandler Handler(&Archive);
	assert(string("the name") == Handler.Name());
}
//------------------------------------------------------------------------------
/**
	BHandler(BMessage* archive)
	@case			archive is valid, but has no field "_name"
	@param	archive	valid BMessage pointer
	@results		BHandler::Name() returns NULL
 */
void TBHandlerTester::BHandler4()
{
	BMessage Archive;
	BHandler Handler(&Archive);
	assert(Handler.Name() == NULL);
}
//------------------------------------------------------------------------------
/**
	BHandler(BMessage* archive)
	@case			archive is NULL
	@param	archive	NULL
	@results		BHandler::Name() returns NULL
	@note			This test is not enabled against the original implementation
					as it doesn't check for a NULL parameter and seg faults.
 */

void TBHandlerTester::BHandler5()
{
#if !defined(SYSTEM_TEST)
	BHandler Handler((BMessage*)NULL);
	assert(Handler.Name() == NULL);
#endif
}
//------------------------------------------------------------------------------
/**
	Archive(BMessage* data, bool deep = true)
	@case			data is NULL, deep is false
	@param	data	NULL
	@param	deep	false
	@results		Returns B_BAD_VALUE
	@note			This test is not enabled against the original implementation
					as it doesn't check for NULL parameters and seg faults
 */
void TBHandlerTester::Archive1()
{
#if !defined(SYSTEM_TEST)
	BHandler Handler;
	assert(Handler.Archive(NULL, false) == B_BAD_VALUE);
#endif
}
//------------------------------------------------------------------------------
/**
	Archive(BMessage* data, bool deep = true)
	@case			data is NULL, deep is true
	@param	data	NULL
	@param	deep	false
	@results		Returns B_BAD_VALUE
	@note			This test is not enabled against the original implementation
					as it doesn't check for NULL parameters and seg faults
 */
void TBHandlerTester::Archive2()
{
#if !defined(SYSTEM_TEST)
	BHandler Handler;
	assert(Handler.Archive(NULL) == B_BAD_VALUE);
#endif
}
//------------------------------------------------------------------------------
/**
	Archive(BMessage* data, bool deep = true)
	@case			data is valid, deep is false
	@param	data	valid BMessage pointer
	@param	deep	false
	@results		Returns B_OK
					Resultant archive has string field labeled "_name"
					Field "_name" contains the string "a name"
					Resultant archive has string field labeled "class"
					Field "class" contains the string "BHandler"
 */
void TBHandlerTester::Archive3()
{
	BMessage Archive;
	BHandler Handler("a name");
	assert(Handler.Archive(&Archive, false) == B_OK);

	const char* data;
	assert(Archive.FindString("_name", &data) == B_OK);
	assert(string("a name") == data);
	assert(Archive.FindString("class", &data) == B_OK);
	assert(string("BHandler") == data);
}
//------------------------------------------------------------------------------
/**
	Archive(BMessage *data, bool deep = true)
	@case			data is valid, deep is true
	@param	data	valid BMessage pointer
	@param	deep	true
	@results		Returns B_OK
					Resultant archive has string field labeled "_name"
					Field "_name" contains the string "a name"
					Resultant archive has string field labeled "class"
					Field "class" contains the string "BHandler"
 */
void TBHandlerTester::Archive4()
{
	BMessage Archive;
	BHandler Handler("another name");
	assert(Handler.Archive(&Archive) == B_OK);

	const char* data;
	assert(Archive.FindString("_name", &data) == B_OK);
	assert(string("another name") == data);
	assert(Archive.FindString("class", &data) == B_OK);
	assert(string("BHandler") == data);
}
//------------------------------------------------------------------------------
/**
	Instantiate(BMessage* data)
	@case			data is NULL
	@param	data	NULL
	@results
	@note			This test is not enabled against the original implementation
					as it doesn't check for NULL parameters and seg faults
 */
void TBHandlerTester::Instantiate1()
{
#if !defined(SYSTEM_TEST)
	assert(BHandler::Instantiate(NULL) == NULL);
	assert(errno == B_BAD_VALUE);
#endif
}
//------------------------------------------------------------------------------
/**
	Instantiate(BMessage* data)
	@case			data is valid, has field "_name"
	@param	data	Valid BMessage pointer with string field "class" containing
					string "BHandler" and with string field "_name" containing
					string "a name"
	@results		BHandler::Name() returns "a name"
 */
void TBHandlerTester::Instantiate2()
{
	BMessage Archive;
	Archive.AddString("class", "BHandler");
	Archive.AddString("_name", "a name");

	BHandler* Handler =
		dynamic_cast<BHandler*>(BHandler::Instantiate(&Archive));
	assert(Handler != NULL);
	assert(string("a name") == Handler->Name());
	assert(errno == B_OK);
}
//------------------------------------------------------------------------------
/**
	Instantiate(BMessage *data)
	@case			data is valid, has no field "_name"
	@param	data	valid BMessage pointer with string field "class" containing
					string "BHandler"
	@results		BHandler::Name() returns NULL
 */

void TBHandlerTester::Instantiate3()
{
	BMessage Archive;
	Archive.AddString("class", "BHandler");

	BHandler* Handler =
		dynamic_cast<BHandler*>(BHandler::Instantiate(&Archive));
	assert(Handler != NULL);
	assert(Handler->Name() == NULL);
	assert(errno == B_OK);
}
//------------------------------------------------------------------------------
/**
	SetName(const char* name)
	Name()
	@case			name is NULL
	@param	name	NULL
	@results		BHandler::Name() returns NULL
	
 */
void TBHandlerTester::SetName1()
{
	BHandler Handler("a name");
	assert(string("a name") == Handler.Name());

	Handler.SetName(NULL);
	assert(Handler.Name() == NULL);
}
//------------------------------------------------------------------------------
/**
	SetName(const char *name)
	Name()
	@case			name is valid
	@param	name	Valid string pointer
	@results		BHandler::Name returns name
 */
void TBHandlerTester::SetName2()
{
	BHandler Handler("a name");
	assert(string("a name") == Handler.Name());

	Handler.SetName("another name");
	assert(string("another name") == Handler.Name());
}
//------------------------------------------------------------------------------
/**
	Perform(perform_code d, void *arg)
	@case		feed meaningless data, should return B_ERROR
	@param	d	N/A
	@param	arg	NULL
	@results	Returns B_ERROR
 */
void TBHandlerTester::Perform1()
{
	BHandler Handler;
	assert(Handler.Perform(0, NULL) == B_ERROR);
}
//------------------------------------------------------------------------------
/**
	IsWatched()
	@case		No added watchers
	@results	Returns false
 */

void TBHandlerTester::IsWatched1()
{
	BHandler Handler;
	assert(Handler.IsWatched() == false);
}
//------------------------------------------------------------------------------
/**
	IsWatched()
	@case		Add then remove watcher
	@results		Returns true after add, returns false after remove
	@note		Original implementation fails this test.  Either the removal
				doesn't happen (unlikely) or some list-within-a-list doesn't
				get removed when there's nothing in it anymore.
 */
void TBHandlerTester::IsWatched2()
{
	BHandler Handler;
	BHandler Watcher;
	Handler.StartWatching(&Watcher, '1234');
	assert(Handler.IsWatched() == true);

	Handler.StopWatching(&Watcher, '1234');
	assert(Handler.IsWatched() == false);
}
//------------------------------------------------------------------------------
/**
	Looper()
	@case		Not added to a BLooper
	@results		Returns NULL
 */
void TBHandlerTester::Looper1()
{
	BHandler Handler;
	assert(Handler.Looper() == NULL);
}
//------------------------------------------------------------------------------
/**
	Looper()
	@case		Add to a BLooper, then remove
	@results	Returns the added-to BLooper; when removed, returns NULL
 */
void TBHandlerTester::Looper2()
{
	BHandler Handler;
	BLooper Looper;
	Looper.AddHandler(&Handler);
	assert(Handler.Looper() == &Looper);

	assert(Looper.RemoveHandler(&Handler));
	assert(Handler.Looper() == NULL);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 do not belong to a BLooper
	@param	handler	Valid BHandler pointer
	@results		NextHandler() returns NULL
					debug message "handler must belong to looper before setting
					NextHandler"
 */
void TBHandlerTester::SetNextHandler1()
{
	BHandler Handler1;
	BHandler Handler2;
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == NULL);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 belongs to a unlocked BLooper, Handler2 does not
	@param	handler	Valid BHandler pointer
	@results		NextHandler() returns BLooper
					debug message "The handler and its NextHandler must have
					the same looper"
 */
void TBHandlerTester::SetNextHandler2()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper;
	Looper.AddHandler(&Handler1);
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 belongs to a locked BLooper, Handler2 does not
	@param	handler	Valid BHandler pointer
	@results		NextHandler() returns BLooper
					debug message "The handler and its NextHandler must have
					the same looper"
 */
void TBHandlerTester::SetNextHandler3()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper;
	Looper.AddHandler(&Handler1);
	Looper.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler2 belongs to a unlocked BLooper, Handler1 does not
	@param	handler	Valid BHandler pointer
	@results		NextHandler() returns NULL
					debug message "handler must belong to looper before setting
					NextHandler"
 */
void TBHandlerTester::SetNextHandler4()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper;
	Looper.AddHandler(&Handler2);
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == NULL);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler2 belongs to a locked BLooper, Handler1 does not
	@param	handler	Valid BHandler pointer
	@results		NextHandler() returns NULL
					debug message "handler must belong to looper before setting
					NextHandler"
 */
void TBHandlerTester::SetNextHandler5()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper;
	Looper.AddHandler(&Handler2);
	Looper.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == NULL);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to different unlocked BLoopers
	@param	handler	Valid BHandler pointer
	@results		Returns BLooper;
					debug message "The handler and its NextHandler must have the
					same looper"
 */
void TBHandlerTester::SetNextHandler6()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper1;
	BLooper Looper2;
	Looper1.AddHandler(&Handler1);
	Looper2.AddHandler(&Handler2);
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper1);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to different BLoopers;
					Handler1's is locked; Handler2's is not
	@param	handler	Valid BHandler pointer
	@result			Returns BLooper;
					debug message "The handler and its NextHandler must have the
					same looper"
 */
void TBHandlerTester::SetNextHandler7()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper1;
	BLooper Looper2;
	Looper1.AddHandler(&Handler1);
	Looper2.AddHandler(&Handler2);
	Looper1.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper1);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to different BLoopers;
					Handler1's is unlocked; Handler2's is locked
	@param	handler	Valid BHandler pointer
	@results		Returns BLooper
					debug message "The handler and its NextHandler must have the
					same looper"
 */
void TBHandlerTester::SetNextHandler8()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper1;
	BLooper Looper2;
	Looper1.AddHandler(&Handler1);
	Looper2.AddHandler(&Handler2);
	Looper2.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper1);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to different locked BLoopers
	@param	handler	Valid BHandler pointer
	@results		Returns BLooper
					debug message "The handler and its NextHandler must have the
					same looper"
 */
void TBHandlerTester::SetNextHandler9()
{
	BHandler Handler1;
	BHandler Handler2;
	BLooper Looper1;
	BLooper Looper2;
	Looper1.AddHandler(&Handler1);
	Looper2.AddHandler(&Handler2);
	Looper1.Lock();
	Looper2.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Looper1);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to the same unlocked BLooper
	@param	handler	Valid BHandler pointer
	@results		Returns Handler2
	@note			Docs say the looper must be locked, but the original
					implementation allows the next handler to be set anyway.
 */
void TBHandlerTester::SetNextHandler10()
{
	BLooper Looper;
	BHandler Handler1;
	BHandler Handler2;
	Looper.AddHandler(&Handler1);
	Looper.AddHandler(&Handler2);
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Handler2);
}
//------------------------------------------------------------------------------
/**
	SetNextHandler(BHandler* handler);
	NextHandler();
	@case			Handler1 and Handler2 belong to the same locked BLooper
	@param	handler	Valid BHandler pointer
	@results		Returns Handler2
 */
void TBHandlerTester::SetNextHandler11()
{
	BLooper Looper;
	BHandler Handler1;
	BHandler Handler2;
	Looper.AddHandler(&Handler1);
	Looper.AddHandler(&Handler2);
	Looper.Lock();
	Handler1.SetNextHandler(&Handler2);
	assert(Handler1.NextHandler() == &Handler2);
}
//------------------------------------------------------------------------------
/**
	NextHandler()
	@case		Default constructed BHandler
	@results	Returns NULL
 */
void TBHandlerTester::NextHandler1()
{
	BHandler Handler;
	assert(Handler.NextHandler() == NULL);
}
//------------------------------------------------------------------------------
/**
	NextHandler();
	@case		Default constructed BHandler added to BLooper
	@results	Returns parent BLooper
 */
void TBHandlerTester::NextHandler2()
{
	BHandler Handler;
	BLooper Looper;
	Looper.AddHandler(&Handler);
	assert(Handler.NextHandler() == &Looper);
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is NULL
	@param	filter	NULL
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					it belongs to a BLooper when a filter gets added.  Also,
					the original implementation does not handle a NULL param
					gracefully, so this test is not enabled against it.
 */
void TBHandlerTester::AddFilter1()
{
#if !defined(SYSTEM_TEST)
	BHandler Handler;
	Handler.AddFilter(NULL);
#endif
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has no looper
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					it belongs to a BLooper when a filter gets added.
 */
void TBHandlerTester::AddFilter2()
{
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper isn't locked
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					if belongs to a BLooper when a filter gets added, or
					whether the looper is locked.
 */
void TBHandlerTester::AddFilter3()
{
	BLooper Looper;
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Looper.AddHandler(&Handler);
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper is locked
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
 */
void TBHandlerTester::AddFilter4()
{
	BLooper Looper;
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Looper.Lock();
	Looper.AddHandler(&Handler);
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is NULL
	@param	filter	NULL
	@results		Returns false
 */
void TBHandlerTester::RemoveFilter1()
{
	BHandler Handler;
	assert(!Handler.RemoveFilter(NULL));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, handler has no looper
	@param	filter	Valid BMessageFilter pointer
	@results		Returns true.  Contrary to documentation, original
					implementation of BHandler doesn't care if it belongs to a
					looper or not.
 */
void TBHandlerTester::RemoveFilter2()
{
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Handler.AddFilter(Filter);
	assert(Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper isn't locked
	@param	filter	Valid BMessageFilter pointer
	@results		Returns true.  Contrary to documentation, original
					implementation of BHandler doesn't care if it belongs to a
					looper or not.
 */
void TBHandlerTester::RemoveFilter3()
{
	BLooper Looper;
	BHandler Handler;
	Looper.AddHandler(&Handler);
	BMessageFilter* Filter = new BMessageFilter('1234');
	Handler.AddFilter(Filter);
	assert(Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper is locked
	@param	filter	Valid BMessageFilter pointer
	@results		Return true.
 */
void TBHandlerTester::RemoveFilter4()
{
	BLooper Looper;
	BHandler Handler;
	Looper.AddHandler(&Handler);
	Looper.Lock();
	BMessageFilter* Filter = new BMessageFilter('1234');
	Handler.AddFilter(Filter);
	assert(Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, but not owned by handler, handler has no looper
	@param	filter	Valid BMessageFilter pointer
	@results		Returns false.  Contrary to documentation, original
					implementation of BHandler doesn't care if it belongs to a
					looper or not.
 */
void TBHandlerTester::RemoveFilter5()
{
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	assert(!Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, but not owned by handler, handler has
					looper, looper isn't locked
	@param	filter	Valid BMessageFilter pointer
	@results		Returns false.  Contrary to documentation, original
					implementation of BHandler doesn't care if its looper is
					locked or not.
 */
void TBHandlerTester::RemoveFilter6()
{
	BLooper Looper;
	BHandler Handler;
	Looper.AddHandler(&Handler);
	BMessageFilter* Filter = new BMessageFilter('1234');
	assert(!Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
/**
	RemoveFilter(BMessageFilter* filter)
	@case			filter is valid, but not owned by handler, handler has
					looper, looper is locked
	@param	filter	Valid BMessageFilter pointer
	@results		Returns false.
 */
void TBHandlerTester::RemoveFilter7()
{
	BLooper Looper;
	BHandler Handler;
	Looper.AddHandler(&Handler);
	Looper.Lock();
	BMessageFilter* Filter = new BMessageFilter('1234');
	assert(!Handler.RemoveFilter(Filter));
}
//------------------------------------------------------------------------------
Test* TBHandlerTester::Suite()
{
	TestSuite* SuiteOfTests = new TestSuite;

	ADD_TEST(SuiteOfTests, TBHandlerTester, BHandler1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, BHandler2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, BHandler3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, BHandler4);
	ADD_TEST(SuiteOfTests, TBHandlerTester, BHandler5);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Archive1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Archive2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Archive3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Archive4);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Instantiate1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Instantiate2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Instantiate3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, SetName1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, SetName2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Perform1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, IsWatched1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, IsWatched2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Looper1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Looper2);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler1);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler2);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler3);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler4);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler5);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler6);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler7);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler8);
//	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler9);
	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler10);
	ADD_TEST(SuiteOfTests, TBHandlerTester, SetNextHandler11);
	ADD_TEST(SuiteOfTests, TBHandlerTester, NextHandler1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, NextHandler2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, AddFilter1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, AddFilter2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, AddFilter3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, AddFilter4);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter4);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter5);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter6);
	ADD_TEST(SuiteOfTests, TBHandlerTester, RemoveFilter7);

	return SuiteOfTests;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

