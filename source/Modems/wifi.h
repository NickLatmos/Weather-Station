#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>

#define WIFI_PORT								PORTB
#define WIFI_DDR								DDRB
#define WIFI_PIN								PINB
#define WIFI_BUTTON							PB0			// Input
#define WIFI_WAKE_UP_PIN				PB1

#define MAX_WIFI_CONNECTION_TIMEOUT 15000  // If after 15 seconds WiFi module is still not connected to the router activate GPRS.

#ifdef __cplusplus
extern "C" {
	void check_available_data_from_WiFi_module(void);
	void receive_message_from_WiFi_module(void);
	void WiFi_module_send_request(char *request);
	bool wifi_is_activated(void);
	bool wifi_is_connected_to_router(void);
	void wifi_connection_succedded(bool conn_status);
	void WiFi_functioning(bool f_status);
	bool IsWIFIFunctioning(void);
	void wake_up_wifi_modem(void);
}
#else
	void check_available_data_from_WiFi_module(void);
	void receive_message_from_WiFi_module(void);
	void WiFi_module_send_request(char *request);
	bool wifi_is_activated(void);
	bool wifi_is_connected_to_router(void);
	void wifi_connection_succedded(bool conn_status);
	void WiFi_functioning(bool f_status);
	bool IsWIFIFunctioning(void);
	void wake_up_wifi_modem(void);
#endif

#endif