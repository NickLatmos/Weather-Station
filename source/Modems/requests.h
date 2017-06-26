#ifndef REQUESTS_H
#define REQUESTS_H

#define MAX_REQUEST_HEADER_SIZE 300                     // Maximum request header size
#define MAX_REQUEST_PAYLOAD_SIZE 250                    // Maximum request payload size

typedef enum PAYLOAD_TYPE {
	DATA,
	TIME,
	DATE,
	VALVE,
} payload_t;

typedef enum CALLER {
	WIFI,
	GPRS,
} caller_t;


#ifdef __cplusplus
extern "C" {
	char* build_payload(payload_t type, caller_t caller);
	char* discard_header(char *response, payload_t type);
	char* check_response(char *message);
	char* extract_uint8_t_from_string(uint8_t *p_byte, char *message);
	char* extract_uint16_t_from_string(int *p_int, char *message);
}
#else
	char* build_payload(payload_t type, caller_t caller);
	char* discard_header(char *response, payload_t type);
	char* check_response(char *message);
	char* extract_uint8_t_from_string(uint8_t *p_byte, char *message);
	char* extract_uint16_t_from_string(int *p_int, char *message);
#endif

#endif