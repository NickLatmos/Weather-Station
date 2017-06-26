#include <stdint.h>
#include <string.h>
#include "weather.h"

char* get_sun_status(uint16_t brightness);
char* get_rain_satus(uint16_t rainning);
long map(long x, long in_min, long in_max, long out_min, long out_max);

//According to the sunlight and the rain status make a weather assessment
char* get_weather(uint16_t brightness, uint16_t rainning)
{
	char *sun = get_sun_status(brightness);
	char *rain = get_rain_satus(rainning);
	
	if (!strcmp(rain,"Light rain") && strcmp(sun,"Sunny"))		 return rain;
	else if (!strcmp(rain, "Raining") && strcmp(sun, "Sunny")) return rain;
	else																											 return	sun;
}

//According to the LDR output make an assessment of the weather
char* get_sun_status(uint16_t brightness)
{
	char *sunlight;
	int range = map(brightness, 0, 1023, 0, 100);

	if (range <= 10) sunlight = "Night";
	else if (range <= 75) sunlight = "Clouds";
	else sunlight = "Sunny";
	
	return sunlight;
}

// Get rain status
char* get_rain_satus(uint16_t rainning)
{
	char *rain_status;
	int range = map(rainning, 0, 1024, 0, 100);
	
	if (range <= 10) rain_status = "Not Raining";
	else if (range <= 75) rain_status = "Light rain";
	else rain_status = "Raining";
	
	return rain_status;
}

// Map a numeric range onto another
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}