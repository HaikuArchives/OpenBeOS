#ifndef _VOLUME_CONTROL_
#define _VOLUME_CONTROL_

/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: VolumeControl.h
 *  DESCR: transitional private volume control functions
 ***********************************************************************/

namespace media_kit_private {

status_t GetMasterVolume(float *left, float *right);
status_t SetMasterVolume(float left, float right);

} //namespace media_kit_private

#endif
