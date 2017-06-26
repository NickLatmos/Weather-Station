/*
ds18b20 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

// 1 time slot = 60us

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ds18b20.h"

/*
 * ds18b20 init
 */
uint8_t ds18b20_reset() {
	uint8_t i;

	//low for 480us -- indicates a reset
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	//The slave must hold it low otherwise no slave found 
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

	//return the read value, 0=ok, 1=error
	return i;
}

/*
 * write one bit
 * 1us-15us set to LOW: inidcates a "1"
 * 60-120 set to LOW: indicates a "0" 
 */
void ds18b20_writebit(uint8_t bit){
	//low for 1uS-15us
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(2);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ); //input

	//wait 60uS (one time slot) and release the line
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}

/*
 * read one bit
 */
uint8_t ds18b20_readbit(void){
	uint8_t bit=0;

	//low for 1uS-15us
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(2);

	//release line and wait for 15uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(15);

	//read the value
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

/*
 * write one byte
 */
void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
uint8_t ds18b20_readbyte(void){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}

/*
 * get temperature
 */
float ds18b20_gettemp() {
	uint8_t temperature_l;
	uint8_t temperature_h;
	float retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion

	while(!ds18b20_readbit()); //wait until conversion is complete

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	//read scratchpad. This command can be issued after SKIP ROM only if there is just one slave device on the bus
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); 

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte();  //data[0]
	temperature_h = ds18b20_readbyte();  //data[1]

	ds18b20_reset(); //terminate reading if only part of the scratchpad data is needed

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	//convert the 12 bit value obtained
	retd = ( ( temperature_h << 8 ) + temperature_l ) * 0.0625;

	return retd;
}