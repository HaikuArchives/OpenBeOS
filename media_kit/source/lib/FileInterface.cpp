/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: FileInterface.cpp
 *  DESCR: 
 ***********************************************************************/
#include <FileInterface.h>
#include "debug.h"

/*************************************************************
 * protected BFileInterface
 *************************************************************/

BFileInterface::~BFileInterface()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BFileInterface
 *************************************************************/

/* nothing */

/*************************************************************
 * protected BFileInterface
 *************************************************************/

BFileInterface::BFileInterface()
	: BMediaNode("XXX fixme")
{
	UNIMPLEMENTED();
}


status_t
BFileInterface::HandleMessage(int32 message,
							  const void *data,
							  size_t size)
{
	UNIMPLEMENTED();

	return B_OK;
}

/*************************************************************
 * private BFileInterface
 *************************************************************/

/*
private unimplemented
BFileInterface::BFileInterface(const BFileInterface &clone)
FileInterface & BFileInterface::operator=(const BFileInterface &clone)
*/

status_t BFileInterface::_Reserved_FileInterface_0(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_1(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_2(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_3(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_4(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_5(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_6(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_7(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_8(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_9(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_10(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_11(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_12(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_13(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_14(void *) { return 0; }
status_t BFileInterface::_Reserved_FileInterface_15(void *) { return 0; }

