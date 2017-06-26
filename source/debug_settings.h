#ifndef DEBUG_SETTINGS_H
#define DEBUG_SETTINGS_H

#define DEBUG_WIFI 0
#define DEBUG_GPRS 0	 

//uart0 is used for sensor debugging
#define DEBUG_BMP180			0
#define DEBUG_DHT22				0
#define DEBUG_DS18B20			0
#define DEBUG_LDR					0
#define DEBUG_RAIN				0
#define DEBUG_WEATHER			0
#define DEBUG_FLOW_RATE		0
#define DEBUG_MILLILTRES	0
#define DEBUG_BATTERY			0

// Can't debug both at the same time
#if DEBUG_WIFI && DEBUG_GPRS	
#undef DEBUG_GPRS
#define DEBUG_GPRS 0
#endif

#if DEBUG_GPRS || DEBUG_WIFI
#include "Protocols/uart.h"										// used in debugging process
#endif

#endif