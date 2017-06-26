/*
 * weather_station.cpp
 *
 * Created: 28/4/2017 12:41:04 
 * Author : Nick
 */ 

#include "processor.h"

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "bit_manipulations.h"
#include "millis.h"
#include "adc.h"
#include "Sensors/bmp_180.h"
#include "Sensors/dht22.h"
#include "Sensors/ds18b20.h"
#include "Sensors/weather.h"
#include "Sensors/water_flow.h"
#include "Modems/wifi.h"
#include "Modems/gprs.h"
#include "Modems/requests.h"
#include "Protocols/uart.h"
#include "global_variables.h"
#include "debug_settings.h"
#include <avr/wdt.h>

#define VALVE_SAMPLE_TIME											 15000   // Remote Control interval (client asks server if it has a message for it) [milliseconds]
#define SENSORS_SAMPLING_TIME									 150000	 // Data transmission interval [milliseconds]
#define MAXIMUM_WAITING_RESPONSE_TIME					 8500    // If no response arrives during this time then stop waiting [milliseconds]
#define STATUS_LED_TOGGLING_TIME							 1			 // Network Status LED [secs]
#define VALVE_TO_POST_DELAY_TIME							 6000    // Time to post data after receiving valve status [milliseconds]

#if STATUS_LED_TOGGLING_TIME >= 8
#error "STATUS_LED_TOGGLING_TIME must be lower than 8 seconds"
#endif

// Battery input pin
#define BATTERY_INPUT_PIN						PC3
#define BATTERY_DDR									DDRC	
#define BATTERY_PORT								PORTC			
#define BATTERY_PIN									PINC
#define BATTERY_ADC_CHANNEL					3
#define BATTERY_ERROR_MEASUREMENT	  0.08
	
//Indicator 
#define STATUS_DDR									DDRD
#define STATUS_PORT									PORTD
#define STATUS_PIN									PIND
#define CONNECTION_STATUS_LED				PD3

// Wind sensor
/*#define WIND_SENSOR_PIN							PE3
#define WIND_SENSOR_PORT						PORTE
#define WIND_SENSOR_DDR							DDRE
#define WIND_SENSOR_ADC_CHANNEL			7
*/

// LDR 
#define LDR_PIN											PC0
#define LDR_PORT										PORTC
#define LDR_DDR											DDRC
#define LDR_ADC_CHANNEL							0

// Raindrop sensor
#define RAINDROP_SENSOR_PIN					PE2
#define RAINDROP_SENSOR_PORT				PORTE
#define RAINDROP_SENSOR_DDR					DDRE
#define RAINDROP_SENSOR_ADC_CHANNEL 6

// Valve Control Pin
#define VALVE_CONTROL_PIN						PD4
#define VALVE_CONTROL_PORT					PORTD
#define VALVE_CONTROL_DDR						DDRD

void set_status_led_toggling(void);
void stop_status_led_toggling(void);
void read_sensors(void);
void check_valve_status(void);
void check_if_system_is_paused(unsigned long current_time);

bool static STATUS_LED_TOGGLING = true;

// Global variables 
const char HOST_URL[] = "alatmos.dyndns.org";
const char DATA_PORT[] = "5000";
const char VALVE_PORT[] = "5500";
const char ID[] = "XYZ";

BMP180 bmp;										// Connected to TWI0

int humidity = 0;
int temperature_dht = 0;
float case_temperature = 0;
float temperature_ds = 0;
float altitude = 0;
float flow_rate = 0;
float battery_voltage = 0;
unsigned long total_milliLitres = 0;
int32_t pressure = 0;
uint16_t brightness = 0;
uint16_t rainning = 0;
char *weather = "";
char valve_status[3] = "";					// NO / NC 
	
// Read pages 71-72 in Data sheet for more info
void setup_watchdog_timer(void)
{
	//disable interrupts
	cli();
	//reset watchdog
	wdt_reset();
	//set up WDT
	WDTCSR = (1<<WDCE)|(1<<WDE);
	//Start watchdog timer with 4s prescaler
	WDTCSR = (1<<WDE)|(1<<WDP3)|(1<<WDP0);
	//Enable global interrupts
	sei();
}

// Called only on reset
void setup(void)
{
	BIT_SET(GPRS_CONTROL_DDR, GPRS_CONTROL_PIN);										
	BIT_SET(GPRS_CONTROL_PORT, GPRS_CONTROL_PIN);							// Disable GPRS
	
	BIT_CLEAR(GPRS_INPUT_DDR, GPRS_BUTTON);										// GPRS input (status indicator button [pressed/not pressed])
	 
	BIT_CLEAR(WIFI_DDR, WIFI_BUTTON);													// WiFi input button -- Connected via dip switch
	BIT_SET(WIFI_DDR, WIFI_WAKE_UP_PIN);											// WiFi connected to router (? 0/1)
	BIT_CLEAR(WIFI_PORT, WIFI_WAKE_UP_PIN);										// Wakes on HIGH
	
	BIT_SET(VALVE_CONTROL_DDR, VALVE_CONTROL_PIN);	
	BIT_CLEAR(VALVE_CONTROL_PORT, VALVE_CONTROL_PIN);					// Set valve control pin as output and low
	
	BIT_CLEAR(LDR_DDR,LDR_PIN);																// LDR as input
	BIT_CLEAR(RAINDROP_SENSOR_DDR, RAINDROP_SENSOR_PIN);			// Raindrop sensor as input
	BIT_CLEAR(WATER_FLOW_SENSOR_DDR, WATER_FLOW_SENSE_PIN);		// Water flow sensor as input (interrupt driven an 5 volts)
	//BIT_CLEAR(WIND_SENSOR_DDR, WIND_SENSOR_PIN);						// Anemometer input
	BIT_CLEAR(BATTERY_DDR, BATTERY_INPUT_PIN);								// Battery voltage input
	
	BIT_SET(STATUS_DDR,CONNECTION_STATUS_LED);								// Led indicator as output
	BIT_SET(STATUS_PORT, CONNECTION_STATUS_LED);

	uart0_init_9600();
	//uart1_init_9600();			// Will only be enabled if GPRS is activated
	
	millis_init();
	adc_init();
	setup_water_flow_interrupt();
	
	// initialize BMP180	
	while(!bmp.begin())
	{
		#if DEBUG_BMP180 
			uart0_put_string_P(PSTR("Trying to initialize BMP180...\n"));
		#endif
	}
	
	_delay_ms(2000);					//wait for garbage to arrive and clean them
	uart0_clear_buffer();
	//uart1_clear_buffer();
	
	#if DEBUG_WIFI
		uart1_init_9600();
		uart1_put_string_P(PSTR("Everything is set\n"));		
	#elif DEBUG_GPRS
		uart0_put_string_P(PSTR("Everything is set\n"));		
	#endif
	
	set_status_led_toggling();
	sei();
}

int main(void)
{
	setup_watchdog_timer();
	setup();
	unsigned long current_time = millis();
	unsigned long previous_valve_poll_time = current_time;
	unsigned long previous_water_flow_sensor_time = current_time;
	unsigned long previous_sensor_sample_time = current_time;
			
  while (1) 
  {
		wdt_reset();
		current_time = millis();	
		
		// Update water flow parameters
		if((current_time - previous_water_flow_sensor_time >= 1000) && !strncmp(valve_status, "NO", 2)){
			check_flow_sensor(current_time, previous_water_flow_sensor_time, &flow_rate, &total_milliLitres);
			previous_water_flow_sensor_time = current_time;
		}
	
		// Check if any data available in the serial port
		if(wifi_is_activated()) check_available_data_from_WiFi_module(); 
		if(GSM_GPRS_is_activated()) check_available_data_from_GSM_GPRS_module();

		// Led indicator for connection status (Open -> Connected/ Toggling -> Trying to connect)
		if(wifi_is_connected_to_router() || GSM_GPRS_is_activated())
			stop_status_led_toggling();
		else
			set_status_led_toggling();
			
		// Stop waiting if no response is received after transmitting a request
		check_if_system_is_paused(current_time);
		
		// Ask for valve status
		if((current_time - previous_valve_poll_time >= VALVE_SAMPLE_TIME || strlen(valve_status) == 0) && !IsGPRSFunctioning() && !IsWIFIFunctioning())
		{
			payload_t type = VALVE;
			/*#if DEBUG_WIFI
				uart1_put_string_P(PSTR("Check TCP status for Valve port.\n"));
			#elif DEBUG_GPRS
				uart0_put_string_P(PSTR("Check TCP status for Valve port.\n"));
			#endif*/
		
			if(wifi_is_connected_to_router())
			{
				caller_t caller = WIFI;
				char *request = build_payload(type, caller);
				wake_up_wifi_modem();
				WiFi_module_send_request(request);	
			}else if(GSM_GPRS_is_activated())
			{
				caller_t caller = GPRS;
				char *request = build_payload(type, caller);
				bool send_request;
				// If valve status not available ask for it.
				if(strlen(valve_status) == 0) send_request = true;
				else send_request = false;
				GPRS_send_request_to_server(VALVE_CONN_NUMBER, HOST_URL, VALVE_PORT, request, send_request);  // If already connected do not send request
			}
			previous_valve_poll_time = millis();
			//valve_to_post_threshold_time = current_time;
		}
		
		check_valve_status();
		
		// Post weather data
		if((current_time - previous_sensor_sample_time >= SENSORS_SAMPLING_TIME || strlen(weather) == 0) && !IsGPRSFunctioning() && !IsWIFIFunctioning())
		{
			payload_t type = DATA;
			#if DEBUG_WIFI
				uart1_put_string_P(PSTR("Sensor Sampling\n"));
			#elif DEBUG_GPRS
				uart0_put_string_P(PSTR("Sensor Sampling\n"));
			#endif

			if(wifi_is_connected_to_router())
			{
				#if DEBUG_WIFI
					uart1_put_string_P(PSTR("Connected to Router\n"));
				#endif
				read_sensors();   // important to be here. 
				caller_t caller = WIFI;
				char *request = build_payload(type, caller);
				wake_up_wifi_modem();
				WiFi_module_send_request(request);
			}else if(GSM_GPRS_is_activated())
			{
				read_sensors();
				caller_t caller = GPRS;
				char *request = build_payload(type, caller); 
				GPRS_send_request_to_server(DATA_CONN_NUMBER, HOST_URL, DATA_PORT, request, true);
			}
			previous_sensor_sample_time = current_time;
		}
		
  }
}

// If server doesn't response for a specific period of time (MAXIMUM_WAITING_RESPONSE_TIME) 
// then let the system continue its normal execution instead of waiting
void check_if_system_is_paused(unsigned long current_time)
{
  static bool FLAG_FIRST_TIME_PAUSED = true;
  static unsigned long time_system_is_paused = 0;
  
  if(IsGPRSFunctioning() || IsWIFIFunctioning())
  {
	  // Save the time when checking for the first time
	  if(FLAG_FIRST_TIME_PAUSED)
	  {
		  FLAG_FIRST_TIME_PAUSED = false;
		  time_system_is_paused = current_time;
	  }
	  else
	  {
		  // Reset variables if modem waits for a response for more than 3 seconds
		  if(current_time - time_system_is_paused >= MAXIMUM_WAITING_RESPONSE_TIME)
		  {
			  GPRS_functioning(false);
			  WiFi_functioning(false);
			  FLAG_FIRST_TIME_PAUSED = true;
		  }
	  }
  }else FLAG_FIRST_TIME_PAUSED = true;   //Data have been received from the server so reset the flag	
}

// Check valve status -- Can be inserted in requests.c
void check_valve_status(void)
{
  if (!strncmp(valve_status, "NO", 2)) BIT_SET(VALVE_CONTROL_PORT, VALVE_CONTROL_PIN);			// NO opens the valve
  if (!strncmp(valve_status, "NC", 2)) BIT_CLEAR(VALVE_CONTROL_PORT, VALVE_CONTROL_PIN);    // NC closes the valve	
}

// Read every connected sensor and save their data
void read_sensors(void)
{
	dhtxxconvert(DHTXX_DHT22, &DHT_PORT, &DHT_DDR, &DHT_PIN, ( 1 << DHT_DATA_PIN ) );  //	Start conversion
	_delay_ms(1000);
	dhtxxread(DHTXX_DHT22, &DHT_PORT, &DHT_DDR, &DHT_PIN, ( 1 << DHT_DATA_PIN ), &temperature_dht, &humidity ); // Get data
	temperature_dht /= 10;
	humidity /= 10;
	
	case_temperature = roundf(bmp.readTemperature());
	pressure = bmp.readPressure();
	altitude = bmp.readAltitude();
	temperature_ds = roundf(ds18b20_gettemp());
	brightness = adc_read(LDR_ADC_CHANNEL);
	rainning = adc_read(RAINDROP_SENSOR_ADC_CHANNEL);
	weather = get_weather(brightness, rainning);
	
	/*
	 To measure the battery voltage: 
	 We know that 3.3V ADC input corresponds to 1023 ADC value
	 I have taken 2 measurements using a voltage divider R2/(R1+R2) where R1 = 10k, R2 = 3.3K:
	 Point A: (2.569V, 794)  (Vin = 10.30V)
	 Point B: (2.799V, 866)	 (Vin = 11.22V)
	 y = 313x - 10.10  (assumed linear relationship)
	 For a random measurement I got an ADC value of 815 which corresponds to 2.636V
	 which in turn corresponds to 2.636*(R1+R2)/R2 = 10.62V. 
	 When I measured the real value it was 10.56 so there was an error of +0.06Volts.
	 To compensate for the error we subtract the error from the battery voltage 
	 */
	uint16_t battery_adc = adc_read(BATTERY_ADC_CHANNEL);
	battery_voltage = (battery_adc + 10.10)/313 * (4.0303) - BATTERY_ERROR_MEASUREMENT;  // 13.3/3.3 = 4.0303
	
	#if DEBUG_BMP180 || DEBUG_DHT22 || DEBUG_DS18B20 || DEBUG_FLOW_RATE || DEBUG_LDR || DEBUG_RAIN || DEBUG_WEATHER || DEBUG_MILLILTRES || DEBUG_BATTERY
		char buf[10];
	#endif
	#if DEBUG_BMP180
		sprintf(buf, "%d", (int) case_temperature);
		uart0_put_string_P(PSTR("Case Temperature: "));
		uart0_put_string(buf);
		uart0_putchar('\n');
		sprintf(buf, "%ld", (int32_t) pressure);
		uart0_put_string_P(PSTR("Pressure\n"));
		uart0_put_string(buf);
		uart0_putchar('\n');
		sprintf(buf, "%d", (int) altitude);
		uart0_put_string_P(PSTR("Altitude\n"));
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_DHT22
		sprintf(buf, "%d", temperature_dht);
		uart0_put_string_P(PSTR("DHT22 temperature\n"));
		uart0_put_string(buf);
		uart0_putchar('\n');	
		sprintf(buf, "%d", humidity);
		uart0_put_string_P(PSTR("DHT22 humidity\n"));
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_DS18B20
		sprintf(buf, "%d", (int) temperature_ds);
		uart0_put_string_P(PSTR("DS18B20 temperature\n"));
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_LDR
		uart0_put_string_P(PSTR("LDR ADC value: "));
		sprintf(buf, "%u", brightness );
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_RAIN
		uart0_put_string_P(PSTR("Rain ADC value:\n"));
		sprintf(buf, "%u", rainning );
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_WEATHER
		uart0_put_string_P(PSTR("Weather:\n"));
		uart0_put_string(weather);
		uart0_putchar('\n');
	#endif
	#if DEBUG_FLOW_RATE
		uart0_put_string_P(PSTR("Flow rate\n"));
		sprintf(buf, "%d", (int) flow_rate);
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_MILLILTRES
		uart0_put_string_P(PSTR("Total millitres:\n"));
		sprintf(buf, "%lu", total_milliLitres );
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
	#if DEBUG_BATTERY
		uart0_put_string_P(PSTR("Battery Voltage:\n"));
		sprintf(buf, "%.2f", battery_voltage );
		uart0_put_string(buf);
		uart0_putchar('\n');
	#endif
}

// Set STATUS_LED_TOGGLING_TIME second toggle time for status led
void set_status_led_toggling(void)
{
	if(STATUS_LED_TOGGLING)
	{
		TCCR4A = 0x00;						// CTC mode
		BIT_SET(TCCR4B, WGM42);		// CTC
		TCNT4 = 0;
		OCR4A = 7813 * STATUS_LED_TOGGLING_TIME - 1; // Max STATUS_LED_TOGGLING_TIME <= 8 secs, fcpu/N ~ 7813
		BIT_SET(TIMSK4, OCIE4A);	// Output compare A match interrupt enable
		BIT_SET(TCCR4B, CS42);		// N = 1024 -- Start clock
		BIT_SET(TCCR4B, CS40);
		STATUS_LED_TOGGLING = false;
	}
}

ISR(TIMER4_COMPA_vect)
{
	BIT_FLIP(STATUS_PORT,CONNECTION_STATUS_LED);
}

// Stop time4 in order to pause led toggling
void stop_status_led_toggling(void)
{
	STATUS_LED_TOGGLING = true;
	TCCR4B = 0x00; 
	BIT_SET(STATUS_PORT, CONNECTION_STATUS_LED);  // Keep it open
}
