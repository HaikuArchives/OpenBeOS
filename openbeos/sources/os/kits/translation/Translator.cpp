/********************************************************************************
/
/      File:           Translator.cpp
/
/      Description:    This header file defines a superclass for translator
/                      objects which you can create within your application and 
/                      then add to a TranslatorRoster.
/
/      Copyright 1998-1999, Be Incorporated, All Rights Reserved.
/      Copyright 1995-1997, Jon Watte
/
/      2002 - Reimplimented by Michael Wilber, OpenBeOS Translation Kit Team
/
********************************************************************************/

#include <Translator.h>

// Set refcount to 1
BTranslator::BTranslator()
{
	fRefCount = 1;
	fSem = create_sem(1, "BTranslator Lock");
}

BTranslator::~BTranslator()
{
	delete_sem(fSem);
	fSem = 0;
}

// Increments the refcount and returns a 
// pointer to this object
BTranslator *BTranslator::Acquire()
{
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		fRefCount++;
		release_sem(fSem);
		return this;
	} else
		return NULL;
}

// decrements the refcount and returns a pointer
// to this object. When the refcount hits zero,
// the object is destroyed and
BTranslator *BTranslator::Release()
{
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		fRefCount--;
		if (fRefCount > 0) {
			release_sem(fSem);
			return this;
		} else {
			~BTranslator();
			return NULL;
		}
	} else
		return NULL;
}

// This function returns the current
// refcount. Notice that it is not thread safe.
// This function is only meant for fun/debugging.
int32 BTranslator::ReferenceCount()
{
	return fRefCount;
}

status_t BTranslator::MakeConfigurationView(BMessage *ioExtension,
	BView **outView, BRect *outExtent)
{
	return B_ERROR;
}

status_t BTranslator::GetConfigurationMessage(BMessage *ioExtension)
{
	return B_ERROR;
}

////////////////////////
// Private Virutal Functions for
// Maintaining binary compatibility
////////////////////////
status_t BTranslator::_Reserved_Translator_0(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_1(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_2(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_3(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_4(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_5(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_6(int32 n, void *p)
{
	return B_ERROR;
}

status_t BTranslator::_Reserved_Translator_7(int32 n, void *p)
{
	return B_ERROR;
}
