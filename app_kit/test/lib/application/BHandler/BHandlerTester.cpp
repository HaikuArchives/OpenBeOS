//------------------------------------------------------------------------------
//	BHandlerTester.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/Message.h>

#ifdef SYSTEM_TEST
#include <be/app/Handler.h>
#else
#include "../../../../source/lib/application/headers/Handler.h"
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
void TBHandlerTester::Case1()
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
void TBHandlerTester::Case2()
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
void TBHandlerTester::Case3()
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
void TBHandlerTester::Case4()
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

void TBHandlerTester::Case5()
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
void TBHandlerTester::Case6()
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
void TBHandlerTester::Case7()
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
void TBHandlerTester::Case8()
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
void TBHandlerTester::Case9()
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
void TBHandlerTester::Case10()
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
void TBHandlerTester::Case11()
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

void TBHandlerTester::Case12()
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
void TBHandlerTester::Case13()
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
void TBHandlerTester::Case14()
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
void TBHandlerTester::Case15()
{
	BHandler Handler;
	assert(Handler.Perform(0, NULL) == B_ERROR);
}
//------------------------------------------------------------------------------
//IsWatched()
//case 1: No added watchers; should return false
//case 2: Add watcher, should return true; remove watcher, should return false

void TBHandlerTester::Case16()
{
}
//------------------------------------------------------------------------------
void TBHandlerTester::Case17()
{
}
//------------------------------------------------------------------------------
Test* TBHandlerTester::Suite()
{
	TestSuite* SuiteOfTests = new TestSuite;

	ADD_TEST(SuiteOfTests, TBHandlerTester, Case1);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case2);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case3);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case4);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case5);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case6);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case7);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case8);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case9);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case10);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case11);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case12);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case13);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case14);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case15);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case16);
	ADD_TEST(SuiteOfTests, TBHandlerTester, Case17);

	return SuiteOfTests;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

