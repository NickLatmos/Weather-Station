/*
 * File Description:
 * 
 * This file contains the functions used to initialize, send and receive a character 
 * with UART. For the moment only USART0 has been configured but the MCU has one more
 * which might be used in the future.
 */
#include "../processor.h"

#include <avr/io.h>  
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "../millis.h"
#include "../bit_manipulations.h"
#include "uart.h"

/*
 * This function initializes the UART operations.
 */
void uart0_init_9600(void)
{
	#undef BAUD
	#define BAUD 9600
	#include <util/setbaud.h>											// Calculates the desired baud rate
	UBRR0H = UBRRH_VALUE;													// Set baud rate registers
	UBRR0L = UBRRL_VALUE;
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);								// Enable receiver and transmitter	UCSR0C = (0<<UMSEL00)|(0<<USBS0)|(3<<UCSZ00); // Asynchronous mode, 1 stop bit, 8 data bits, no parity bit
	UCSR0B |= (1 << RXCIE0);											// Enable the USART Recieve Complete interrupt (USART_RXC)
}

void uart1_init_9600(void)
{
	#undef BAUD
	#define BAUD 2400
	#include <util/setbaud.h>										  // Calculates the desired baud rate
	UBRR1H = UBRRH_VALUE;													// Set baud rate registers
	UBRR1L = UBRRL_VALUE;
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);								// Enable receiver and transmitter	UCSR1C = (0<<UMSEL10)|(0<<USBS1)|(3<<UCSZ10); // Asynchronous mode, 1 stop bit, 8 data bits, no parity bit
}

void disable_uart1(void)
{
  UCSR1B = 0x00;  UCSR1C = 0x00;	
}

// Send a string 
void uart0_put_string(const char *request)
{
	while(*request){
		uart0_putchar(*request);
		request++;
	}
	BIT_SET(UCSR0A,TXC0);													// Buffer is empty
}

void uart1_put_string(const char *request)
{
	while(*request){
		uart1_putchar(*request);
		request++;
	}
	BIT_SET(UCSR1A,TXC1);													// Buffer is empty
}

// Send a string stored in flash memory
void uart0_put_string_P(const char *request)
{
	while(pgm_read_byte(request)){ uart0_putchar(pgm_read_byte(request++)); }
	BIT_SET(UCSR0A,TXC0);													// Buffer is empty
}

void uart1_put_string_P(const char *request)
{
	while(pgm_read_byte(request)){ uart1_putchar(pgm_read_byte(request++)); }
	BIT_SET(UCSR1A,TXC1);													// Buffer is empty
}


// Transmit a character when data register is empty.
void uart0_putchar(char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0); 
	UDR0 = c;
}

void uart1_putchar(char c)
{
	loop_until_bit_is_set(UCSR1A, UDRE1); 
	UDR1 = c;
}

/*
 * @delay_time indicates the maximum number of ms to wait if not receiving any character
 * If no character arrives return '#'
 */
char uart0_getchar(uint16_t delay_time)
{
	unsigned long now = millis();
	while ( !(UCSR0A & (1<<RXC0)) )
	{
		if(millis() - now >= delay_time)
		  return '#';  
	} 
	return UDR0;
}

char uart1_getchar(uint16_t delay_time)
{
	unsigned long now = millis();
	while ( !(UCSR1A & (1<<RXC1)) )
	{
		if(millis() - now >= delay_time) 
			return '#'; 
	}
	return UDR1;
}

// Wait until data exists.
char uart0_getchar_blocking(void) 
{
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

char uart1_getchar_blocking(void)
{
	loop_until_bit_is_set(UCSR1A, RXC1); 
	return UDR1;
}

// While there are unread data read them
void uart0_clear_buffer(void)
{
	while ( (UCSR0A & (1<<RXC0)) )
	{
		char trash = UDR0;
		loop_until_bit_is_set(UCSR0A,UDRE0); 
	}
}

void uart1_clear_buffer(void)
{
	while ( (UCSR1A & (1<<RXC1)) )
	{
		char trash = UDR1;
		loop_until_bit_is_set(UCSR1A,UDRE1); 
	}
}

// Check if there are any data in the buffer
bool uart0_available_data(void)
{
	if(BIT_READ(UCSR0A,RXC0)) return 1;
	else return 0;
}

bool uart1_available_data(void)
{
	if(BIT_READ(UCSR1A,RXC1)) return 1;
	else return 0;
}
