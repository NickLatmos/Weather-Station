#ifndef UART_H
#define UART_H

#include <stdint.h>  //include uint8_t ...
#include <stdbool.h>

#ifdef __cplusplus
	extern "C" {
		void uart0_init_9600(void);
		void uart0_put_string(const char *request);
		void uart0_put_string_P(const char *request);
		void uart0_putchar(char c);
		char uart0_getchar(uint16_t delay_time);
		char uart0_getchar_blocking(void);
		void uart0_clear_buffer(void);
		bool uart0_available_data(void);

		void uart1_init_9600(void); 
		void uart1_put_string(const char *request);
		void uart1_put_string_P(const char *request);
		void uart1_putchar(char c);
		char uart1_getchar(uint16_t delay_time);
		char uart1_getchar_blocking(void);
		void uart1_clear_buffer(void);
		bool uart1_available_data(void);
		void disable_uart1(void);
	}
#else
		void uart0_init_9600(void);
		void uart0_put_string(const char *request);
		void uart0_put_string_P(const char *request);
		void uart0_putchar(char c);
		char uart0_getchar(uint16_t delay_time);
		char uart0_getchar_blocking(void);
		void uart0_clear_buffer(void);
		bool uart0_available_data(void);

		void uart1_init_9600(void);
		void uart1_put_string(const char *request);
		void uart1_put_string_P(const char *request);
		void uart1_putchar(char c);
		char uart1_getchar(uint16_t delay_time);
		char uart1_getchar_blocking(void);
		void uart1_clear_buffer(void);
		bool uart1_available_data(void);
		void disable_uart1(void);
#endif

#endif