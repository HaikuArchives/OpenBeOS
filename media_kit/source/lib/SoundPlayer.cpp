/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SoundPlayer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <SoundPlayer.h>
#include "debug.h"

/*************************************************************
 * public sound_error
 *************************************************************/

sound_error::sound_error(const char *str)
{
	UNIMPLEMENTED();
}


const char *
sound_error::what() const
{
	UNIMPLEMENTED();
	return NULL;
}

/*************************************************************
 * public BSoundPlayer
 *************************************************************/

BSoundPlayer::BSoundPlayer(const char * name,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	UNIMPLEMENTED();
}

BSoundPlayer::BSoundPlayer(const media_raw_audio_format * format, 
						   const char * name,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	UNIMPLEMENTED();
}

BSoundPlayer::BSoundPlayer(const media_node & toNode,
						   const media_multi_audio_format * format,
						   const char * name,
						   const media_input * input,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	UNIMPLEMENTED();
}

/* virtual */
BSoundPlayer::~BSoundPlayer()
{
	UNIMPLEMENTED();
}


status_t
BSoundPlayer::InitCheck()
{
	UNIMPLEMENTED();

	return B_OK;
}


media_raw_audio_format
BSoundPlayer::Format() const
{
	UNIMPLEMENTED();
	media_raw_audio_format dummy;

	return dummy;
}


status_t
BSoundPlayer::Start()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BSoundPlayer::Stop(bool block,
				   bool flush)
{
	UNIMPLEMENTED();
}


BSoundPlayer::BufferPlayerFunc
BSoundPlayer::BufferPlayer() const
{
	UNIMPLEMENTED();
	BufferPlayerFunc dummy;

	return dummy;
}

void BSoundPlayer::SetBufferPlayer(void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format))
{
	UNIMPLEMENTED();
}


BSoundPlayer::EventNotifierFunc
BSoundPlayer::EventNotifier() const
{
	UNIMPLEMENTED();
	EventNotifierFunc dummy;

	return dummy;
}

void BSoundPlayer::SetNotifier(void (*Notifier)(void *, sound_player_notification what, ...))
{
	UNIMPLEMENTED();
}


void *
BSoundPlayer::Cookie() const
{
	UNIMPLEMENTED();
	return NULL;
}


void
BSoundPlayer::SetCookie(void *cookie)
{
	UNIMPLEMENTED();
}

void BSoundPlayer::SetCallbacks(void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
								void (*Notifier)(void *, sound_player_notification what, ...),
								void * cookie)
{
	UNIMPLEMENTED();
}


bigtime_t
BSoundPlayer::CurrentTime()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


bigtime_t
BSoundPlayer::PerformanceTime()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


status_t
BSoundPlayer::Preroll()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


BSoundPlayer::play_id
BSoundPlayer::StartPlaying(BSound *sound,
						   bigtime_t at_time)
{
	UNIMPLEMENTED();
	play_id dummy;

	return dummy;
}
 

BSoundPlayer::play_id
BSoundPlayer::StartPlaying(BSound *sound,
						   bigtime_t at_time,
						   float with_volume)
{
	UNIMPLEMENTED();
	play_id dummy;

	return dummy;
}


status_t
BSoundPlayer::SetSoundVolume(play_id sound,
							 float new_volume)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bool
BSoundPlayer::IsPlaying(play_id id)
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


status_t
BSoundPlayer::StopPlaying(play_id id)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BSoundPlayer::WaitForSound(play_id id)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


float
BSoundPlayer::Volume()
{
	UNIMPLEMENTED();
	float dummy;

	return dummy;
}


void
BSoundPlayer::SetVolume(float new_volume)
{
	UNIMPLEMENTED();
}


float
BSoundPlayer::VolumeDB(bool forcePoll)
{
	UNIMPLEMENTED();
	float dummy;

	return dummy;
}


void
BSoundPlayer::SetVolumeDB(float volume_dB)
{
	UNIMPLEMENTED();
}


status_t
BSoundPlayer::GetVolumeInfo(media_node *out_node,
							int32 *out_parameter,
							float *out_min_dB,
							float *out_max_dB)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bigtime_t
BSoundPlayer::Latency()
{
	UNIMPLEMENTED();
	bigtime_t dummy;

	return dummy;
}


/* virtual */ bool
BSoundPlayer::HasData()
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


void
BSoundPlayer::SetHasData(bool has_data)
{
	UNIMPLEMENTED();
}


/*************************************************************
 * protected BSoundPlayer
 *************************************************************/

void
BSoundPlayer::SetInitError(status_t in_error)
{
	UNIMPLEMENTED();
}


/*************************************************************
 * private BSoundPlayer
 *************************************************************/

status_t BSoundPlayer::_Reserved_SoundPlayer_0(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_1(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_2(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_3(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_4(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_5(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_6(void *, ...) { return 0; }
status_t BSoundPlayer::_Reserved_SoundPlayer_7(void *, ...) { return 0; }


void
BSoundPlayer::NotifySoundDone(play_id sound,
							  bool got_to_play)
{
	UNIMPLEMENTED();
}


void
BSoundPlayer::get_volume_slider()
{
	UNIMPLEMENTED();
}


/* virtual */ void
BSoundPlayer::Notify(sound_player_notification what,
					 ...)
{
	UNIMPLEMENTED();
}


/* virtual */ void
BSoundPlayer::PlayBuffer(void *buffer,
						 size_t size,
						 const media_raw_audio_format &format)
{
	UNIMPLEMENTED();
}


