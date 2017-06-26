/*
 ** Copyright https://github.com/g4lvanix/I2C-master-lib **
 */
#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdint.h>
#include <stdbool.h>

#define I2C_READ 0x01
#define I2C_WRITE 0x00

#ifdef __cplusplus
extern "C"{
	void i2c0_init(void);
	uint8_t i2c0_start(uint8_t address);
	uint8_t i2c0_write(uint8_t data);
	uint8_t i2c0_read_ack(void);
	uint8_t i2c0_read_nack(void);
	uint8_t i2c0_transmit(uint8_t address, uint8_t* data, uint16_t length);
	uint8_t i2c0_receive(uint8_t address, uint8_t* data, uint16_t length);
	uint8_t i2c0_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
	uint8_t i2c0_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
	bool i2c0_read_uint16_register(uint8_t devaddr, uint8_t regaddr, uint16_t *data);
	void i2c0_stop(void);
}
#else
	void i2c0_init(void);
	uint8_t i2c0_start(uint8_t address);
	uint8_t i2c0_write(uint8_t data);
	uint8_t i2c0_read_ack(void);
	uint8_t i2c0_read_nack(void);
	uint8_t i2c0_transmit(uint8_t address, uint8_t* data, uint16_t length);
	uint8_t i2c0_receive(uint8_t address, uint8_t* data, uint16_t length);
	uint8_t i2c0_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
	uint8_t i2c0_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
	bool i2c0_read_uint16_register(uint8_t devaddr, uint8_t regaddr, uint16_t *data);
	void i2c0_stop(void);
#endif
#endif 