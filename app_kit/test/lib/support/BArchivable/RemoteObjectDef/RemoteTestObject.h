//------------------------------------------------------------------------------
//	RemoteTestObject.h
//
//------------------------------------------------------------------------------

#ifndef REMOTETESTOBJECT_H
#define REMOTETESTOBJECT_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#ifdef SYSTEM_TEST
#include <be/support/Archivable.h>
#else
#include "../../../../../source/lib/support/headers/Archivable.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class BMessage;

class TRemoteTestObject : public BArchivable
{
	public:
		TRemoteTestObject(int32 i);
		int32 GetData() { return data; }

		// All the archiving-related stuff
		TRemoteTestObject(BMessage* archive);
		status_t Archive(BMessage* archive, bool deep = true);
		static TRemoteTestObject* Instantiate(BMessage* archive);

	private:
		int32 data;
};

#endif	//REMOTETESTOBJECT_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

