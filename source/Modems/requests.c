#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>
#include <avr/pgmspace.h>
#include "requests.h"
#include "../bit_manipulations.h"
#include "../global_variables.h"
#include "../debug_settings.h"
#include "../Protocols/uart.h"

extern void wifi_connection_succedded(bool conn_status);
extern void WiFi_module_send_request(char *request);

static char header[MAX_REQUEST_HEADER_SIZE];
static char payload[MAX_REQUEST_PAYLOAD_SIZE];

uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;
uint8_t day = 0;
uint8_t month = 0;
int year = 0;

/*
 * The request to send (header + payload)
 */
char* build_payload(payload_t type, caller_t caller)
{
  uint16_t content_length = 0;
	char temp[10];
	
	// Reset previous requests
	memset(header, '\0', MAX_REQUEST_HEADER_SIZE);		
	memset(payload, '\0', MAX_REQUEST_PAYLOAD_SIZE);
	
  switch(type)
  { 
    case DATA:
      //header
      strcpy_P(header, PSTR("POST /weather_station/ HTTP/1.1\r\n"));
      if(caller == WIFI) strcat_P(header, PSTR("Host: 192.168.1.60:5000\r\n"));      // this must be changed to alatmos.dyndns.org
			else							 strcat_P(header, PSTR("Host: alatmos.dyndns.org:5000\r\n"));
      strcat_P(header, PSTR("Content-Type: application/x-www-form-urlencoded\r\n"));
       
      //body
      strcpy_P(payload,PSTR("ID="));
      strcat(payload, ID);
      strcat_P(payload, PSTR("&temperature="));
      sprintf(temp, "%d", (int) temperature_ds);       //sprintf in embedded system does not work with float numbers, so convert it to an integer
      strcat(payload, temp);
      strcat_P(payload, PSTR("&humidity="));
      sprintf(temp, "%d", (int) humidity);
      strcat(payload, temp);
      strcat_P(payload, PSTR("&pressure="));
      sprintf(temp, "%lu", pressure);                  // C89 version. in C99 try zu (doesn't work here)
      strcat(payload, temp);
      strcat_P(payload, PSTR("&case_temperature="));
      sprintf(temp, "%d", (int) case_temperature);
      strcat(payload, temp);
      strcat_P(payload,PSTR("&weather="));
      strcat(payload, weather);
			strcat_P(payload, PSTR("&battery="));				//&humidity=30&pressure=100130&case_temperature=32&weather=Clouds
			sprintf(temp, "%.2f", battery_voltage);
			strcat(payload, temp);
					
      //add content length to header for the post method
      strcat_P(header, PSTR("Content-Length: "));
      content_length = strlen(payload);
      sprintf(temp, "%d\r\n", content_length);
      strcat(header, temp);
      strcat_P(header, PSTR("\r\n"));  
      break;
    
    //ask the server for the current time
    case TIME:
      strcpy_P(header,PSTR("GET /time HTTP/1.1\r\n"));
      //strcat(header, "Host: 192.168.1.20:5000\r\n");
      strcat_P(header,PSTR("Host: alatmos.dyndns.org:5000\r\n"));
      strcat_P(header, PSTR("\r\n"));
      break;

    //ask the server for the current date
    case DATE:
      strcpy_P(header,PSTR("GET /date HTTP/1.1\r\n"));
      //strcat(header, "Host: 192.168.1.20:5000\r\n");
      strcat_P(header,PSTR("Host: alatmos.dyndns.org:5000\r\n"));
      strcat_P(header,PSTR("\r\n"));  
      break;
      
    case VALVE:
      strcpy_P(header, PSTR("VALVE?ID="));
      strcat(header, ID);  
      break;
      
    default:
      break;
  } 
	
	char *final_request = header;
	strcat(final_request, payload);
	return final_request;
}

/*Get rid of the header*/
char* discard_header(char *response, payload_t type)
{
	char *message = 0;
	
  message = strstr(response, "WiFi"); 
  if(message == NULL) message = strstr(response, "VALVE");
	//if(message == NULL) message = strstr(response, "HTTP");
  if(message == NULL) message = strstr(response, "TIME");
  if(message == NULL) message = strstr(response, "DATE");
  
  return message;
}

/*
 * Response format:
 * 
 * For the valve: VALVE NO/NC \a
 * For the time:  TIME HH:MM:SS \a
 * For the date:  DATE YYYY:MM:DD \a
 */
char* check_response(char *message)
{
  if(!strncmp(message, "VALVE", strlen("VALVE"))){
    char temp[3];
    temp[0] = message[6];
    temp[1] = message[7];
    // Precautions in case of garbage
    if(!strncmp(temp, "NO", 2) || !strncmp(temp, "NC", 2)){
      valve_status[0] = message[6];
      valve_status[1] = message[7];
    }
  }else if(!strncmp(message, "TIME", strlen("TIME"))){
    message = extract_uint8_t_from_string(&hour, message);
    message = extract_uint8_t_from_string(&minute, message);
    message = extract_uint8_t_from_string(&second, message);
  }else if(!strncmp(message, "DATE", strlen("DATE"))){
    message = extract_uint16_t_from_string(&year, message);
    message = extract_uint8_t_from_string(&month, message);
    message = extract_uint8_t_from_string(&day, message); 
  }else if(!strncmp(message, "WiFi Unavailable", strlen("WiFi Unavailable")))
	{
    wifi_connection_succedded(false);
		#if DEBUG_WIFI 
			uart1_put_string("WiFi NOT connected to router"); 
		#endif
  }else if(!strncmp(message, "WiFi OK", strlen("WiFi OK")))
	{
		uart0_put_string_P(PSTR("CONNECTED"));   // Answer which is expected by the modem
		wifi_connection_succedded(true);
		#if DEBUG_WIFI
      uart1_put_string("WiFi connected to router");
		#endif
  }
  return message;
}

//extract a number (uint8_t) from a string
char* extract_uint8_t_from_string(uint8_t *p_byte, char *message)
{
  while(*message){
    if(isdigit(*message)){
      *p_byte = (uint8_t) strtol(message, &message, 10);
      break;
    } 
    message++;
  }
  return message;
}

//extract a number (uint16_t) from a string
char* extract_uint16_t_from_string(int *p_int, char *message)
{
  while(*message){
    if(isdigit(*message)){
      *p_int = (int) strtol(message, &message, 10);
      break;
    } 
    message++;
  }
  return message;
}