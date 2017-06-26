#ifndef GPRS_H
#define GPRS_H

#include <stdint.h>
#include <stdbool.h>

//#define COSMOTE_SIM
#define VODAFONE_SIM

#define GPRS_CONTROL_PIN	PB2
#define GPRS_CONTROL_PORT	PORTB
#define GPRS_CONTROL_DDR	DDRB

#define GPRS_BUTTON				PD6
#define GPRS_INPUT_PORT		PORTD
#define GPRS_INPUT_DDR		DDRD
#define GPRS_INPUT_PIN		PIND

#define GPRS_MODULE_RESPONSE_LENGTH						64
#define NUMBER_OF_FAILED_CONNECTION_FOR_RESET 2

const char VALVE_CONN_NUMBER = '1';
const char DATA_CONN_NUMBER = '2';

extern bool wifi_is_connected_to_router(void);
extern bool wifi_is_activated(void);
extern void check_available_data_from_WiFi_module(void);

#ifdef __cplusplus
extern "C" {
	bool setup_GSM_GPRS_Module(void);
	//void send_AT_command_to_GSM_GPRS(const char *command);
	void setup_wireless_connection(void);
	void shutdown_wireless_connection(void);
	bool GSM_GPRS_start_TCP_connection(char conn_num, const char *host, const char *port, char *expected_response, int threshold_time);
	void GSM_GPRS_close_TCP_connection(char conn_num, char *expected_response, int threshold_time);
	void GSM_GPRS_send_request(char conn_num, char *request, bool module_status);
	void check_available_data_from_GSM_GPRS_module(void);
	void read_GSM_GPRS_server_response(void);
	bool read_GSM_GPRS_module_response(char *expected_response, int threshold_time);
	bool check_server_connection_status(char conn_num, char *expected_response, int threshold_time);
	void GPRS_functioning(bool f_status);
	bool IsGPRSFunctioning(void);
	bool GSM_GPRS_is_activated(void);
	void reset_GPRS_module(void);
	void GSM_GPRS_flush(void);
	void GPRS_send_request_to_server(char conn_num, const char *host, const char *port, char *request, bool send_if_already_connected);
	void open_GPRS(void);
	void close_GPRS(void);
}
#else
	bool setup_GSM_GPRS_Module(void);
	//void send_AT_command_to_GSM_GPRS(const char *command);
	void setup_wireless_connection(void);
	void shutdown_wireless_connection(void);
	bool GSM_GPRS_start_TCP_connection(char conn_num, const char *host, const char *port, char *expected_response, int threshold_time);
	void GSM_GPRS_close_TCP_connection(char conn_num, char *expected_response, int threshold_time);
	void GSM_GPRS_send_request(char conn_num, char *request, bool module_status);
	void check_available_data_from_GSM_GPRS_module(void);
	void read_GSM_GPRS_server_response(void);
	bool read_GSM_GPRS_module_response(char *expected_response, int threshold_time);
	bool check_server_connection_status(char conn_num, char *expected_response, int threshold_time);
	void GPRS_functioning(bool f_status);
	bool IsGPRSFunctioning(void);
	bool GSM_GPRS_is_activated(void);
	void reset_GPRS_module(void);
	void GSM_GPRS_flush(void);
	void GPRS_send_request_to_server(char conn_num, const char *host, const char *port, char *request, bool send_if_already_connected);
	void open_GPRS(void);
	void close_GPRS(void);
#endif

#endif