/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: TrackReader.h
 *  DESCR: The undocumented BTrackReader class, used by BSound
 ***********************************************************************/

#if !defined(_TRACK_READER_H_)
#define _TRACK_READER_H_

#include <MediaTrack.h>

namespace BPrivate
{

class BTrackReader
{
public:
	BTrackReader(BMediaTrack *, media_raw_audio_format const &);
	BTrackReader(BFile *, media_raw_audio_format const &);
	~BTrackReader();
	
	int64 CountFrames(void);
	const media_raw_audio_format & Format(void) const;
	int32 FrameSize(void);
	status_t ReadFrames(void *in_buffer, int32 frame_count);
	status_t SeekToFrame(int64 *in_out_frame);
	BMediaTrack * Track(void);
	
private:
	media_raw_audio_format fFormat;
};

}; //namespace BPrivate

#endif
