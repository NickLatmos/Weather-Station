#include "../processor.h"
#include "wifi.h"
#include "../bit_manipulations.h"
#include "../debug_settings.h"
#include "../Protocols/uart.h"
#include "server.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include <stddef.h>
#include "requests.h"

#if DEBUG_WIFI
#include <avr/pgmspace.h>
#endif

char SERVER_RESPONSE[MAX_SERVER_RESPONSE_SIZE];
int SERVER_RESPONSE_COUNTER = 0;
bool FLAG_SERVER_RESPONSE = false;

static bool FLAG_WIFI_CURRENTLY_ON = false;
static bool FLAG_WIFI_CONNECTION = false;    // If ESP not connected to the WIFi network will return a "WiFi Unavailable" message otherwise "WiFi OK"

// Checks if WiFi module has sent any data.
// If it has take the necessary actions.
void check_available_data_from_WiFi_module(void)
{
	// Check if ESP8266 has some data for the controller
	// Deprecated -- Now used with interrupts
	//if(uart0_available_data()) receive_message_from_WiFi_module();

	//if data have been received from ESP8266 reset the variables
	if (server_response_received())
	{
		char *message;
		
		uart0_clear_buffer();  // new
		
		#if DEBUG_WIFI
			uart1_put_string_P(PSTR("Data received from WiFi: "));
			uart1_put_string(SERVER_RESPONSE);
		#endif
		
		message = discard_header(SERVER_RESPONSE, 0);
		
		#if DEBUG_WIFI
			uart1_put_string_P(PSTR("After discarding the header message: "));
			uart1_put_string(message);
		#endif
		
		message = check_response(message);
		
		reset_server_variables_after_response_received();
		WiFi_functioning(false);
	}
}

/*
 * All the data coming from server must end with '\a'
 */
void receive_message_from_WiFi_module(void)
{
  //receive the data
  while(uart0_available_data())
	{
    if(SERVER_RESPONSE_COUNTER < MAX_SERVER_RESPONSE_SIZE)
      SERVER_RESPONSE[SERVER_RESPONSE_COUNTER++] = uart0_getchar_blocking();
    else{
			server_response_ready(true);
      uart0_clear_buffer();
      break;  
    }
  }

  //response always ends with \a
  if(SERVER_RESPONSE[SERVER_RESPONSE_COUNTER - 1] == '\a')
	{
    server_response_ready(true);
    uart0_clear_buffer();
  }
}

/*
 * Send the request as in REST API.
 */
void WiFi_module_send_request(char* request)
{
  //send payload
  uart0_put_string(request);
  uart0_putchar('\a');
	WiFi_functioning(true);
}

// Returns the WiFi modem status set manually by the user via DIP switch(1 -> ON, 0 -> OFF)
bool wifi_is_activated(void)
{
	if(BIT_READ(WIFI_PIN, WIFI_BUTTON)) return 1;

	wifi_connection_succedded(false);    //reset the WiFi connection in case it is deactivated
	return 0;
}

// Checks if WiFi is connected to the router
inline bool wifi_is_connected_to_router(void)
{
	// The led red on SIM900 consumes 3mA when uart is enabled so close it.
	// If in debugging mode then leave it.
	#if DEBUG_WIFI || DEBUG_GPRS
		return FLAG_WIFI_CONNECTION;
	#else
		// Disable uart if wifi is connected.
		if(FLAG_WIFI_CONNECTION)
			disable_uart1();			
		return FLAG_WIFI_CONNECTION;
	#endif
	//return BIT_READ(WIFI_PIN, WIFI_WAKE_UP_PIN);
}

// Returns whether the WiFi modem successfully connected to the access point
void wifi_connection_succedded(bool conn_status)
{
	FLAG_WIFI_CONNECTION = conn_status;
}

//GPRS is working thus not available for other purposes
void WiFi_functioning(bool f_status)
{
	FLAG_WIFI_CURRENTLY_ON = f_status;
}

//Check if GPRS is working on something (delivering/sending data)
bool IsWIFIFunctioning(void)
{
	return FLAG_WIFI_CURRENTLY_ON;
}

void wake_up_wifi_modem(void)
{
	BIT_SET(WIFI_PORT, WIFI_WAKE_UP_PIN);
	_delay_ms(10);
	BIT_CLEAR(WIFI_PORT, WIFI_WAKE_UP_PIN);
}

// UART0 interrupt when a character arrives
ISR(USART0_RX_vect)
{
	//response always ends with \a
	if(SERVER_RESPONSE_COUNTER < MAX_SERVER_RESPONSE_SIZE)
	{
		SERVER_RESPONSE[SERVER_RESPONSE_COUNTER++] = UDR0;
		if(SERVER_RESPONSE[SERVER_RESPONSE_COUNTER - 1] == '\a')
			server_response_ready(true);
	}else 
	  server_response_ready(true);
}