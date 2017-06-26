#ifndef WEATHER_H
#define WEATHER_H
#endif

#ifdef __cplusplus
	extern "C" char* get_weather(uint16_t brightness, uint16_t rainning);
#else
	char* get_weather(uint16_t brightness, uint16_t rainning);
#endif