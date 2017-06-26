/* dhtxx.h - a part of avr-dhtxx library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.	See the LICENSE file for details.
 */

#ifndef DHT22_H
#define DHT22_H
#endif

#include <inttypes.h>

#define DHT_PORT		 PORTB
#define DHT_DDR			 DDRB
#define DHT_PIN		   PINB
#define DHT_DATA_PIN PB5

#define DHTXX_ERROR_OK 0
#define DHTXX_ERROR_COMM 1
#define DHTXX_ERROR_CHECKSUM 2
#define DHTXX_ERROR_OTHER 3
#define DHTXX_OK DHTXX_ERROR_OK

#define DHTXX_DHT11 0
#define DHTXX_DHT22 1

#define DHTXX_MUL 10

#ifdef __cplusplus
	extern "C" uint8_t dhtxxread( unsigned char dev, volatile uint8_t *port, volatile uint8_t *direction, volatile uint8_t *portin, uint8_t mask, int *temperature, int *humidity );
	extern "C" uint8_t dhtxxconvert( unsigned char dev, volatile uint8_t *port, volatile uint8_t *direction, volatile uint8_t *portin, uint8_t mask );
#else
	uint8_t dhtxxread( unsigned char dev, volatile uint8_t *port, volatile uint8_t *direction, volatile uint8_t *portin, uint8_t mask, int *temperature, int *humidity );
	uint8_t dhtxxconvert( unsigned char dev, volatile uint8_t *port, volatile uint8_t *direction, volatile uint8_t *portin, uint8_t mask );
#endif