/* net_device.h */

#ifndef NET_DEVICE_H
#define NET_DEVICE_H

typedef struct device_module_info {
	module_info module;
	
	int (*init)(void);
} device_module_info;

#endif /* NET_DEVICE_H */

