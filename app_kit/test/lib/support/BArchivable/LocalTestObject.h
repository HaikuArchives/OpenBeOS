//------------------------------------------------------------------------------
//	LocalTestObject.h
//
//------------------------------------------------------------------------------

#ifndef LOCALTESTOBJECT_H
#define LOCALTESTOBJECT_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/Message.h>
#ifdef SYSTEM_TEST
#include <be/support/Archivable.h>
#else
#include "../../../../source/lib/support/headers/Archivable.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TIOTest : public BArchivable
{
	public:
		TIOTest(int32 i);
		int32 GetData() { return data; }

		// All the archiving-related stuff
		TIOTest(BMessage* archive);
		status_t Archive(BMessage* archive, bool deep = true);
		static TIOTest* Instantiate(BMessage* archive);

	private:
		int32 data;
};

#endif	//LOCALTESTOBJECT_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

