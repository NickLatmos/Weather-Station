#include "../processor.h"
#include "gprs.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include <avr/wdt.h>
#include "requests.h"
#include "server.h"
#include "../bit_manipulations.h"
#include "../millis.h"
#include "../Protocols/uart.h"
#include "../debug_settings.h"

static void send_AT_command_to_GSM_GPRS(const char *command);

//static bool FLAG_SERVER_DATA_READY = false;     //indicates that data have arrived to the main controller from the GPRS module.
static bool FLAG_GPRS_CURRENTLY_ON = false;     //indicates if GPRS module transmits/receives any data.
static uint8_t TCP_FAILED_CONNECTIONS = 0;

/*
 * Settings for the GSM/GPRS A6 module
 */
bool setup_GSM_GPRS_Module(void)
{
	bool flag;
	
	uart1_init_9600();			// enable uart1
	
	wdt_reset();
	#if DEBUG_GPRS
	  uart0_put_string_P(PSTR("Setting up GPRS\n"));
	#endif
	
	close_GPRS();
	_delay_ms(2000);
	open_GPRS();
  _delay_ms(4000);

  GSM_GPRS_flush();
  wdt_reset();
	
  //Disable echo
  send_AT_command_to_GSM_GPRS(PSTR("ATE0"));
  read_GSM_GPRS_module_response("OK",1000);
  
  //Baud rate 57600
  send_AT_command_to_GSM_GPRS(PSTR("AT+IPR=2400"));
  if(!read_GSM_GPRS_module_response("OK",2000))
    return 0;                                     //Reset it fast so that we don't waste time here
  
  //set pin code
  send_AT_command_to_GSM_GPRS(PSTR("AT+CPIN=\"6334\""));
  read_GSM_GPRS_module_response("CREG",13000);        //don't really care
  
  //Print network status
  send_AT_command_to_GSM_GPRS(PSTR("AT+CREG?"));
  read_GSM_GPRS_module_response("CREG",2000);
  
  //Attach to a Packet Domain Service
  send_AT_command_to_GSM_GPRS(PSTR("AT+CGATT=1"));
  if(!read_GSM_GPRS_module_response("OK",5000))
    return 0;

  //Close if any wireless connection
  shutdown_wireless_connection();

  //Multiple Network links
  send_AT_command_to_GSM_GPRS(PSTR("AT+CIPMUX=1"));
  if(!read_GSM_GPRS_module_response("OK",3000))
    return 0;

  //Connect to the network
  setup_wireless_connection();

  //IP address
  send_AT_command_to_GSM_GPRS(PSTR("AT+CIFSR"));
  flag = read_GSM_GPRS_module_response(".",1500);
	
	//Signal quality
  send_AT_command_to_GSM_GPRS(PSTR("AT+CSQ"));
  read_GSM_GPRS_module_response("OK", 2000);
	
  return flag;
}

//Send AT command to the GPRS module followed by an \r
static void send_AT_command_to_GSM_GPRS(const char *command)
{
  uart1_put_string_P(command);
  uart1_putchar('\r');   
}

//Connect to the network (Gets an IP address)
void setup_wireless_connection(void)
{
  //Setup APN, user, password
	#ifdef VODAFONE_SIM
		send_AT_command_to_GSM_GPRS(PSTR("AT+CSTT=\"internet.vodafone.gr\",\"\",\"\""));
	#endif
	#ifdef COSMOTE_SIM
		send_AT_command_to_GSM_GPRS(PSTR("AT+CSTT=\"internet\",\"\",\"\""));
	#endif
  read_GSM_GPRS_module_response("OK",3000);

  //Start wireless connection
  send_AT_command_to_GSM_GPRS(PSTR("AT+CIICR"));
  read_GSM_GPRS_module_response("OK",6000);
}

//Close if any wireless connection
void shutdown_wireless_connection(void)
{
  send_AT_command_to_GSM_GPRS("AT+CIPSHUT");
  read_GSM_GPRS_module_response("SHUT",4000);
}

/*
 * Start TCP connection to Host: host at Port: port
 */
bool GSM_GPRS_start_TCP_connection(char conn_num, const char *host,const char *port, char *expected_response, int threshold_time)
{
  uart1_put_string_P(PSTR("AT+CIPSTART="));
  uart1_putchar(conn_num);
  uart1_put_string_P(PSTR(",\"TCP\",\""));
  uart1_put_string(host);
  uart1_put_string_P(PSTR("\",\""));
  uart1_put_string(port);
  uart1_put_string_P(PSTR("\""));
  uart1_putchar('\r');
  return read_GSM_GPRS_module_response(expected_response, threshold_time);
}

/*Close TCP connection*/
void GSM_GPRS_close_TCP_connection(char conn_num, char *expected_response, int threshold_time)
{
  //When multiple link enabled we must close a specific channel! (tcp_connection_number holds the channel used for REST purposes) 
  uart1_put_string_P(PSTR("AT+CIPCLOSE="));
  uart1_putchar(conn_num);
  uart1_putchar('\r');    
  read_GSM_GPRS_module_response(expected_response, threshold_time);
}

/*
 * Send request to the server. -- Insert a DELAY() before calling this function
 */
void GSM_GPRS_send_request(char conn_num, char *request, bool module_status)
{
  uart1_put_string_P(PSTR("AT+CIPSEND="));
  uart1_putchar(conn_num);
  uart1_putchar('\r');
  read_GSM_GPRS_module_response(">",800);
  //returns a character > at this point to indicate that it's ready to receive the message 
  uart1_put_string(request);
  uart1_putchar(0x1A);                  //send CTRL + Z to push the message
  GPRS_functioning(module_status);
	_delay_ms(20);
}

// Establishes a connection to the server and sends the desired request.
void GPRS_send_request_to_server(char conn_num, const char *host, const char *port, char *request, bool send_if_already_connected)
{
	// Check if already connected
	if(!check_server_connection_status(conn_num , "CONNECT", 800))
	{
		// Start new TCP connection
		if(GSM_GPRS_start_TCP_connection(conn_num, host, port, "CONNECT",2500))
		{
			GSM_GPRS_send_request(conn_num, request, true);
			TCP_FAILED_CONNECTIONS = 0;													// reset failed connection if successfully established communication with the server.
		}else TCP_FAILED_CONNECTIONS++;
	}else if(send_if_already_connected)  // For Remote Control there is no need to send the message again if the connection is still open. 
		GSM_GPRS_send_request(conn_num, request, true);			  
		
	// If can't connect to the server after successive trials the reset the modem	
	if(TCP_FAILED_CONNECTIONS >= NUMBER_OF_FAILED_CONNECTION_FOR_RESET){
		reset_GPRS_module();
		TCP_FAILED_CONNECTIONS = 0;
	}
}

void check_available_data_from_GSM_GPRS_module(void)
{
  // If data arrive without sending a request it's probably from a server response
  //read response from server. Slightly different than the read_GSM_GPRS_module_response().
  if(uart1_available_data()) read_GSM_GPRS_server_response();

  if(server_response_received())
  {
		char *message;
		#if DEBUG_GPRS
			uart0_put_string("-*-Data have been Received-*-\n");
			uart0_put_string(SERVER_RESPONSE);
		#endif
		
		message = discard_header(SERVER_RESPONSE, 0);
		
		#if DEBUG_GPRS
			uart0_put_string("After discarding the header message: \n");
			uart0_put_string(message);
		#endif
		
		message = check_response(message);

		//reset variables
		reset_server_variables_after_response_received();
		GPRS_functioning(false);
  }
}

/*
 * Read's server's response. Always end with '\a'.     
 */
void read_GSM_GPRS_server_response(void)
{
  while(uart1_available_data())
	{   
    if (SERVER_RESPONSE_COUNTER < MAX_SERVER_RESPONSE_SIZE)
    {
      char c = uart1_getchar_blocking();
      if(c == '\0')
        continue;
      SERVER_RESPONSE[SERVER_RESPONSE_COUNTER++] = c;
    }else{ 
      SERVER_RESPONSE[SERVER_RESPONSE_COUNTER] = '\0';
			uart1_clear_buffer();
      break;
    }

    // Indicates the end of the response ('\a' for the VALVE, '}' for the post method)
    if (SERVER_RESPONSE[SERVER_RESPONSE_COUNTER - 1] == '}' || SERVER_RESPONSE[SERVER_RESPONSE_COUNTER - 1] == '\a' )   
    {  
      server_response_ready(true);
      SERVER_RESPONSE[SERVER_RESPONSE_COUNTER] = '\0';
      break;
    }  
  }
}

/*
 * Reads the module's responses and NOT the server's
 */
bool read_GSM_GPRS_module_response(char *expected_response, int threshold_time)
{
  char response[GPRS_MODULE_RESPONSE_LENGTH];
  int counter = 0;
  bool return_flag = false;
  unsigned long int start = millis();

  while(1){
		wdt_reset();
    if(millis() - start >= threshold_time)
		{
			#if DEBUG_GPRS
				uart0_put_string_P(PSTR("\n**Time waiting is over.Break**\n"));
      #endif
			break;  
    }
    if(uart1_available_data())
		{
      while(uart1_available_data())
			{
        if(counter < GPRS_MODULE_RESPONSE_LENGTH)
        {
          char c = uart1_getchar_blocking();
          if(c == '\0')
            continue;
          response[counter++] = c; 
        }else break; 
      }  
    }   
  }

  response[counter] = '\0';
	#if DEBUG_GPRS
		uart0_put_string(response);
		uart0_put_string_P(PSTR("\n\nExpected Response: \n"));
		uart0_put_string(expected_response);
	#endif
	
  if(strstr(response, expected_response) != NULL){
		#if DEBUG_GPRS
			uart0_put_string_P(PSTR("\nFOUND IT\n"));  
		#endif
		return_flag = true;
  }else{
		#if DEBUG_GPRS
			uart0_put_string_P(PSTR("\nNOT FOUND\n"));
		#endif
	}

  memset(response,'\0',GPRS_MODULE_RESPONSE_LENGTH);
  return return_flag;
}

//Returns true if the connection <conn_num> is CONNECTED to the server otherwise false
bool check_server_connection_status(char conn_num, char *expected_response, int threshold_time)
{
  uart1_put_string_P(PSTR("AT+CIPSTATUS="));
  uart1_putchar(conn_num);
  uart1_putchar('\r');
  return read_GSM_GPRS_module_response(expected_response, threshold_time);
}

//GPRS is working thus not available for other purposes
void GPRS_functioning(bool f_status)
{
  FLAG_GPRS_CURRENTLY_ON = f_status;  
}

//Check if GPRS is working on something (delivering/sending data)
bool IsGPRSFunctioning(void)
{
  return FLAG_GPRS_CURRENTLY_ON;  
}

// GPRS must be set on ON by the DIP switch AND the WiFi must not be connected to the router
// in order to be considered as activated.
bool GSM_GPRS_is_activated(void)
{
  static bool FIRST_TIME_FLAG_SETUP = true;
	uint8_t error_counter = 0;
				
	_delay_ms(1);			// Without this delay reading the GPRS_BUTTON returns false values. No idea why...
	
  // Setup the GPRS if the user has activated it and WiFi module is not connected to router.
  if(BIT_READ(GPRS_INPUT_PIN,GPRS_BUTTON) && FIRST_TIME_FLAG_SETUP && !wifi_is_connected_to_router())
	{
		FIRST_TIME_FLAG_SETUP = false;
		#if DEBUG_GPRS
			uart0_put_string_P(PSTR("Setting up GPRS after button press\n"));
		#elif DEBUG_WIFI
			uart1_put_string_P(PSTR("Setting up GPRS after button press\n"));
		#endif
    while(1)
		{
			if(wifi_is_activated()) check_available_data_from_WiFi_module();  // Check if a message was sent from WiFi
			if(wifi_is_connected_to_router())					return 0;								// If the message from wifi modem was "WiFi OK" then turn off GPRS.
			if(!BIT_READ(GPRS_INPUT_PIN,GPRS_BUTTON)) return 0;
			if(setup_GSM_GPRS_Module())								return 1;
			error_counter++;
			if(error_counter >= 3)										return 0;
    } 
  }

  // Turn on the GPRS if user has activated it, WiFi module is not connected to the Internet and the modem must not be in a reset mode.
  if(BIT_READ(GPRS_INPUT_PIN, GPRS_BUTTON) && !wifi_is_connected_to_router())
	{
    BIT_CLEAR(GPRS_CONTROL_PORT,GPRS_CONTROL_PIN);
    return 1;
  }

  // If user deactivates it take care to setup the modem in the next power up.
  if(!BIT_READ(GPRS_INPUT_PIN, GPRS_BUTTON))
    FIRST_TIME_FLAG_SETUP = true;
  
  BIT_SET(GPRS_CONTROL_PORT,GPRS_CONTROL_PIN);
  return 0; 
}

void reset_GPRS_module(void)
{
	int8_t error_counter = 0;
	#if DEBUG_GPRS
		uart0_put_string_P(PSTR("Reset GPRS modem\n"));
	#endif
	while(1)
	{
		if(wifi_is_activated()) check_available_data_from_WiFi_module();  // Check if a message was sent from WiFi
		if(wifi_is_connected_to_router())						break;									// If the message from wifi modem was "WiFi OK" then turn off GPRS.
		if(!BIT_READ(GPRS_INPUT_PIN, GPRS_BUTTON))  break;
		if(setup_GSM_GPRS_Module())									break;
		error_counter++;
		if(error_counter >= 5)											break;
		#if DEBUG_GPRS
			uart0_put_string_P(PSTR("Reset trial #"));
			uart0_putchar((error_counter+'0'));
			uart0_putchar('\n');
		#endif
	}
}

void close_GPRS(void)
{
	BIT_SET(GPRS_CONTROL_PORT, GPRS_CONTROL_PIN);
}

void open_GPRS(void)
{
	BIT_CLEAR(GPRS_CONTROL_PORT, GPRS_CONTROL_PIN);				// Active low
}

void GSM_GPRS_flush(void)
{
  uart1_clear_buffer();
}

