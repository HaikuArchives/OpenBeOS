/******************************************************************************
/
/	File:			DataIO.h
/
/	Description:	Pure virtual BDataIO and BPositioIO classes provide
/					the protocol for Read()/Write()/Seek().
/
/					BMallocIO and BMemoryIO classes implement the protocol,
/					as does BFile in the Storage Kit.
/
/	Copyright 1993-98, Be Incorporated
/
/	History :
/		2002-01-23  Steve Vallée
/					First modifications to use this header for OpenBeOS.
/					- Removed R3 compatibility flags
/					- Removed some unused private methods (non-virtual)
/					- Added {} at the right of all non-implemented virtual methods
/
******************************************************************************/

#ifndef	_DATA_IO_H
#define	_DATA_IO_H

#include <BeBuild.h>
#include <SupportDefs.h>
