/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SoundPlayNode.cpp
 *  DESCR: This is the BBufferProducer, used internally by BSoundPlayer
 *         This belongs into a private namespace, but isn't for 
 *         compatibility reasons.
 ***********************************************************************/

#include <BufferProducer.h>
#include "SoundPlayNode.h"


_SoundPlayNode::_SoundPlayNode(const char *name, const media_multi_audio_format *format) : 
	BBufferProducer(B_MEDIA_RAW_AUDIO),
	BMediaNode(name)
{
	fFormat = *format;
}


_SoundPlayNode::~_SoundPlayNode()
{
}


media_multi_audio_format 
_SoundPlayNode::Format() const
{
	return fFormat;
}


/* virtual */ status_t 
_SoundPlayNode::FormatSuggestionRequested(
				media_type type,
				int32 quality,
				media_format * format)
{
	return B_ERROR;
}


/* virtual */ status_t 
_SoundPlayNode::FormatProposal(
				const media_source & output,
				media_format * format)
{
	return B_ERROR;
}

				
/* If the format isn't good, put a good format into *io_format and return error */
/* If format has wildcard, specialize to what you can do (and change). */
/* If you can change the format, return OK. */
/* The request comes from your destination sychronously, so you cannot ask it */
/* whether it likes it -- you should assume it will since it asked. */
/* virtual */ status_t 
_SoundPlayNode::FormatChangeRequested(
				const media_source & source,
				const media_destination & destination,
				media_format * io_format,
				int32 * _deprecated_)
{
	return B_ERROR;
}

		
/* virtual */ status_t 
_SoundPlayNode::GetNextOutput(	/* cookie starts as 0 */
				int32 * cookie,
				media_output * out_output)
{
	return B_ERROR;
}

				
/* virtual */ status_t 
_SoundPlayNode::DisposeOutputCookie(
				int32 cookie)
{
	return B_ERROR;
}

				
/* In this function, you should either pass on the group to your upstream guy, */
/* or delete your current group and hang on to this group. Deleting the previous */
/* group (unless you passed it on with the reclaim flag set to false) is very */
/* important, else you will 1) leak memory and 2) block someone who may want */
/* to reclaim the buffers living in that group. */
/* virtual */ status_t 
_SoundPlayNode::SetBufferGroup(
				const media_source & for_source,
				BBufferGroup * group)
{
	return B_ERROR;
}

				
/* Format of clipping is (as int16-s): <from line> <npairs> <startclip> <endclip>. */
/* Repeat for each line where the clipping is different from the previous line. */
/* If <npairs> is negative, use the data from line -<npairs> (there are 0 pairs after */
/* a negative <npairs>. Yes, we only support 32k*32k frame buffers for clipping. */
/* Any non-0 field of 'display' means that that field changed, and if you don't support */
/* that change, you should return an error and ignore the request. Note that the buffer */
/* offset values do not have wildcards; 0 (or -1, or whatever) are real values and must */
/* be adhered to. */
/* virtual */ status_t 
_SoundPlayNode::VideoClippingChanged(
				const media_source & for_source,
				int16 num_shorts,
				int16 * clip_data,
				const media_video_display_info & display,
				int32 * _deprecated_)
{
	return B_ERROR;
}

				
/* Iterates over all outputs and maxes the latency found */
/* virtual */ status_t 
_SoundPlayNode::GetLatency(
				bigtime_t * out_lantency)
{
	return B_ERROR;
}

				
/* virtual */ status_t 
_SoundPlayNode::PrepareToConnect(
				const media_source & what,
				const media_destination & where,
				media_format * format,
				media_source * out_source,
				char * out_name)
{
	return B_ERROR;
}

				
/* virtual */ void 
_SoundPlayNode::Connect(
				status_t error, 
				const media_source & source,
				const media_destination & destination,
				const media_format & format,
				char * io_name)
{
}

			
/* virtual */ void 
_SoundPlayNode::Disconnect(
				const media_source & what,
				const media_destination & where)
{
}

			
/* virtual */ void 
_SoundPlayNode::LateNoticeReceived(
				const media_source & what,
				bigtime_t how_much,
				bigtime_t performance_time)
{
}

			
/* virtual */ void 
_SoundPlayNode::EnableOutput(
				const media_source & what,
				bool enabled,
				int32 * _deprecated_)
{
}


/* Who instantiated you -- or NULL for app class */
/* virtual */ BMediaAddOn* 
_SoundPlayNode::AddOn(int32 * internal_id) const
{
	return NULL;
}
