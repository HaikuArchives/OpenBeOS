/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: TrackReader.cpp
 *  DESCR: The undocumented BTrackReader class, used by BSound
 ***********************************************************************/
 
#include <MediaTrack.h>
#include <MediaFile.h>
#include "TrackReader.h"
#include "debug.h"

namespace BPrivate
{

BTrackReader::BTrackReader(BMediaTrack *, media_raw_audio_format const &)
{
	UNIMPLEMENTED();
}

BTrackReader::BTrackReader(BFile *, media_raw_audio_format const &)
{
	UNIMPLEMENTED();
}

BTrackReader::~BTrackReader()
{
	UNIMPLEMENTED();
}

int64 
BTrackReader::CountFrames(void)
{
	UNIMPLEMENTED();
	return 0;
}

const media_raw_audio_format & 
BTrackReader::Format(void) const
{
	UNIMPLEMENTED();
	return fFormat;
}

int32 
BTrackReader::FrameSize(void)
{
	UNIMPLEMENTED();
	return 4;
}

status_t 
BTrackReader::ReadFrames(void *in_buffer, int32 frame_count)
{
	UNIMPLEMENTED();
	return B_OK;
}

status_t 
BTrackReader::SeekToFrame(int64 *in_out_frame)
{
	UNIMPLEMENTED();
	return B_OK;
}

BMediaTrack * 
BTrackReader::Track(void)
{
	UNIMPLEMENTED();
	return 0;
}

}; //namespace BPrivate
