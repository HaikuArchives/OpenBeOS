#ifndef AUDIODEVICEDETECTION
#define AUDIODEVICEDETECTION





#include <String.h>
#include <StorageKit.h>
#include <List.h>





namespace DeviceDetection
	{
	int FindDevices(BList *DeviceList); 
/*	DeviceDetection::FindDevices returns

		A: the count of devices found
		B: the list itself with the device paths
	
	the paths are stored as BStrings
	The parameter is a BList that must have been allocated previously by you
*/
	}





#endif