/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: SoundPlayer.cpp
 *  DESCR: 
 ***********************************************************************/
#include <SoundPlayer.h>
#include <MediaRoster.h>
#include "debug.h"
#include "SoundPlayNode.h"

/*************************************************************
 * public sound_error
 *************************************************************/

//final
sound_error::sound_error(const char *str)
{
	CALLED();
	m_str_const = str;
}

//final
const char *
sound_error::what() const
{
	CALLED();
	return m_str_const;
}

/*************************************************************
 * public BSoundPlayer
 *************************************************************/

BSoundPlayer::BSoundPlayer(const char * name,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	CALLED();
	media_multi_audio_format fmt = media_multi_audio_format::wildcard;
	fmt.frame_rate = 44100.0f;
	fmt.channel_count = 2;
	fmt.format = media_raw_audio_format::B_AUDIO_FLOAT;
	fmt.byte_order = B_MEDIA_HOST_ENDIAN;
	//fmt.raw.buffer_size = 4096;

	Init(NULL,&fmt,name,NULL,PlayBuffer,Notifier,cookie);
}

BSoundPlayer::BSoundPlayer(const media_raw_audio_format * format, 
						   const char * name,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	CALLED();
	media_multi_audio_format fmt = media_multi_audio_format::wildcard;
	memcpy(&fmt,format,sizeof(*format));
	Init(NULL,&fmt,name,NULL,PlayBuffer,Notifier,cookie);
}

BSoundPlayer::BSoundPlayer(const media_node & toNode,
						   const media_multi_audio_format * format,
						   const char * name,
						   const media_input * input,
						   void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
						   void (*Notifier)(void *, sound_player_notification what, ...),
						   void * cookie)
{
	CALLED();
	if (toNode.kind & B_BUFFER_CONSUMER == 0)
		debugger("BSoundPlayer: toNode must have B_BUFFER_CONSUMER kind!\n");
	Init(&toNode,format,name,input,PlayBuffer,Notifier,cookie);
}

/* virtual */
BSoundPlayer::~BSoundPlayer()
{
	UNIMPLEMENTED();
}


status_t
BSoundPlayer::InitCheck()
{
	CALLED();
	return _m_init_err;
}


media_raw_audio_format
BSoundPlayer::Format() const
{
	CALLED();

	media_raw_audio_format temp = media_raw_audio_format::wildcard;
	
	if (_m_node)
		memcpy(&temp,&_m_node->Format(),sizeof(temp));

	return temp;
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
	CALLED();
	return _PlayBuffer;
}

void BSoundPlayer::SetBufferPlayer(void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format))
{
	CALLED();
	_m_lock.Lock();
	_PlayBuffer = PlayBuffer;
	_m_lock.Unlock();
}

BSoundPlayer::EventNotifierFunc
BSoundPlayer::EventNotifier() const
{
	CALLED();
	return _Notifier;
}

void BSoundPlayer::SetNotifier(void (*Notifier)(void *, sound_player_notification what, ...))
{
	CALLED();
	_m_lock.Lock();
	_Notifier = Notifier;
	_m_lock.Unlock();
}

void *
BSoundPlayer::Cookie() const
{
	CALLED();
	return _m_cookie;
}

void
BSoundPlayer::SetCookie(void *cookie)
{
	CALLED();
	_m_lock.Lock();
	_m_cookie = cookie;
	_m_lock.Unlock();
}

void BSoundPlayer::SetCallbacks(void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
								void (*Notifier)(void *, sound_player_notification what, ...),
								void * cookie)
{
	CALLED();
	_m_lock.Lock();
	SetBufferPlayer(PlayBuffer);
	SetNotifier(Notifier);
	SetCookie(cookie);
	_m_lock.Unlock();
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
	BROKEN();
	return 50000;
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

//final
void
BSoundPlayer::SetInitError(status_t in_error)
{
	CALLED();
	_m_init_err = in_error;
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

void 
BSoundPlayer::Init(
					const media_node * node,
					const media_multi_audio_format * format, 
					const char * name,
					const media_input * input,
					void (*PlayBuffer)(void *, void * buffer, size_t size, const media_raw_audio_format & format),
					void (*Notifier)(void *, sound_player_notification what, ...),
					void * cookie)
{
	BROKEN();
	_m_node = NULL;
	_m_sounds = NULL;
	_m_waiting = NULL;
	_PlayBuffer = PlayBuffer;
	_Notifier = Notifier;
	//_m_lock;
	_m_volume = 0.0f;
	//m_input;
	//m_output;
	_m_mix_buffer = 0;
	_m_mix_buffer_size = 0;
	_m_cookie = cookie;
	_m_buf = NULL;
	_m_bufsize = 0;
	_m_has_data = 0;
	_m_init_err = B_ERROR;
	_m_perfTime = 0;
	_m_volumeSlider = NULL;
	_m_gotVolume = 0;

	_m_node = new _SoundPlayNode(name,format);
	
	status_t status; 
	BMediaRoster *roster;
	media_node outnode;
	roster = BMediaRoster::Roster();
	if (!roster) {
		TRACE("BSoundPlayer::Init: Couldn't get BMediaRoster\n");
		return;
	}
	
	//connect our producer node either to the 
	//system mixer or to the supplied out node
	if (!node) {
		status = roster->GetAudioMixer(&outnode);
		if (status != B_OK) {
			TRACE("BSoundPlayer::Init: Couldn't GetAudioMixer\n");
			SetInitError(status);
			return;
		}
		node = &outnode;
	}

/*
	m_input = ;
	m_output = ;

	
tryFormat = fileAudioOutput.format; 
   err = roster->Connect(fileAudioOutput.source, audioInput.destination, 
               &tryFormat, &m_output, &m_input); 


 err = roster->GetStartLatencyFor(timeSourceNode, &startTime); 
   startTime += b_timesource->PerformanceTimeFor(BTimeSource::RealTime() 
               + 1000000 / 50); 
    
   err = roster->StartNode(mediaFileNode, startTime); 
   err = roster->StartNode(codecNode, startTime); 
   err = roster->StartNode(videoNode, startTime); 

*/	
	
	SetInitError(B_OK);
}

/* virtual */ void
BSoundPlayer::Notify(sound_player_notification what,
					 ...)
{
	CALLED();
	_m_lock.Lock();
	if (_Notifier)
		(*_Notifier)(_m_cookie,what);
	else {
	}
	_m_lock.Unlock();
}


/* virtual */ void
BSoundPlayer::PlayBuffer(void *buffer,
						 size_t size,
						 const media_raw_audio_format &format)
{
	CALLED();
	_m_lock.Lock();
	if (_PlayBuffer)
		(*_PlayBuffer)(_m_cookie,buffer,size,format);
	else {
	}
	_m_lock.Unlock();
}


