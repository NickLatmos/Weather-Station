/*
 ** Copyright https://github.com/g4lvanix/I2C-master-lib **
 */
#include "../processor.h"

#include <avr/io.h>
#include "modified_twi.h"
#include "twi_registers.h"
#include "i2c_master.h"

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c0_init(void)
{
	TWBR0 = (uint8_t)TWBR_val;
}

uint8_t i2c0_start(uint8_t address)
{
	// Address Packet format 9 bits. 
	//First 7: address, next R/W and the last ACK/NACK set by the slave
	
	// reset TWI control register
	TWCR0 = 0;
	// transmit START condition
	TWCR0 = (1<<TWINT0) | (1<<TWSTA0) | (1<<TWEN0);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT0)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR0 & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR0 = address;
	// start transmission of address
	TWCR0 = (1<<TWINT0) | (1<<TWEN0);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT0)) );
	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW0_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	
	return 0;
}

uint8_t i2c0_write(uint8_t data)
{
	// load data into data register
	TWDR0 = data;
	// start transmission of data
	TWCR0 = (1<<TWINT0) | (1<<TWEN0);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT0)) );
	
	if( (TWSR0 & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	return 0;
}

uint8_t i2c0_read_ack(void)
{
	// start TWI module and acknowledge data after reception
	TWCR0 = (1<<TWINT0) | (1<<TWEN0) | (1<<TWEA0);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT0)) );
	// return received data from TWDR
	return TWDR0;
}

uint8_t i2c0_read_nack(void)
{
	// start receiving without acknowledging reception
	TWCR0 = (1<<TWINT0) | (1<<TWEN0);
	// wait for end of transmission
	while( !(TWCR0 & (1<<TWINT0)) );
	// return received data from TWDR
	return TWDR0;
}

uint8_t i2c0_transmit(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c0_start(address | I2C_WRITE)) return 1;
	
	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c0_write(data[i])) return 1;
	}
	
	i2c0_stop();
	
	return 0;
}

uint8_t i2c0_receive(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c0_start(address | I2C_READ)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c0_read_ack();
	}
	data[(length-1)] = i2c0_read_nack();
	
	i2c0_stop();
	
	return 0;
}

uint8_t i2c0_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c0_start(devaddr | 0x00)) return 1;

	i2c0_write(regaddr);

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c0_write(data[i])) return 1;
	}

	i2c0_stop();

	return 0;
}

uint8_t i2c0_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c0_start(devaddr)) return 1;
	
	i2c0_write(regaddr);

	if (i2c0_start(devaddr | 0x01)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c0_read_ack();
	}
	data[(length-1)] = i2c0_read_nack();

	i2c0_stop();

	return 0;
}

bool i2c0_read_uint16_register(uint8_t devaddr, uint8_t regaddr, uint16_t *data)
{
	uint8_t data_regs[2];
	if(i2c0_readReg(devaddr, regaddr, data_regs, 2)) return 1;
	*data = data_regs[0];
	*data <<= 8;
	*data |= data_regs[1];
	return 0;
}

void i2c0_stop(void)
{
	// transmit STOP condition
	TWCR0 = (1<<TWINT0) | (1<<TWEN0) | (1<<TWSTO0);
}