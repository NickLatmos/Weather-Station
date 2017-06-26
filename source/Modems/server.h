#ifndef SERVER_H
#define SERVER_H

#include <string.h>

#define MAX_SERVER_RESPONSE_SIZE 500      // Maximum response size received from server

extern char SERVER_RESPONSE[];
extern int SERVER_RESPONSE_COUNTER;
extern bool FLAG_SERVER_RESPONSE;

static inline bool server_response_received(void)
{
	return FLAG_SERVER_RESPONSE;
}

static inline void server_response_ready(bool status)
{
	FLAG_SERVER_RESPONSE = status;
}

static inline void reset_server_variables_after_response_received(void)
{
	//reset variables
	memset(SERVER_RESPONSE, '\0', MAX_SERVER_RESPONSE_SIZE);
	SERVER_RESPONSE_COUNTER = 0;
	server_response_ready(false);
}

#endif