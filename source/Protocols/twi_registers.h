/*
 * For some reason the TWI register names are not available in avr/io.h
 * so I created this file to include them.
 * 
 * Include this file to use TWI protocol for ATmega328PB
 */
#ifndef TWI_REGISTERS_H
#define TWI_REGISTERS_H

#include <avr/sfr_defs.h>

/*-------  TWI0  --------*/
#define TWBR0 _SFR_MEM8(0xB8)
#define TWBR00 0
#define TWBR01 1
#define TWBR02 2
#define TWBR03 3
#define TWBR04 4
#define TWBR05 5
#define TWBR06 6
#define TWBR07 7

#define TWSR0 _SFR_MEM8(0xB9)
#define TWPS00 0
#define TWPS01 1
#define TWS03 3
#define TWS04 4
#define TWS05 5
#define TWS06 6
#define TWS07 7

#define TWAR0 _SFR_MEM8(0xBA)
#define TWGCE0 0
#define TWA00 1
#define TWA01 2
#define TWA02 3
#define TWA03 4
#define TWA04 5
#define TWA05 6
#define TWA06 7

#define TWDR0 _SFR_MEM8(0xBB)
#define TWD00 0
#define TWD01 1
#define TWD02 2
#define TWD03 3
#define TWD04 4
#define TWD05 5
#define TWD06 6
#define TWD07 7

#define TWCR0 _SFR_MEM8(0xBC)
#define TWIE0 0
#define TWEN0 2
#define TWWC0 3
#define TWSTO0 4
#define TWSTA0 5
#define TWEA0 6
#define TWINT0 7

#define TWAMR0 _SFR_MEM8(0xBD)
#define TWAM00 1
#define TWAM01 2
#define TWAM02 3
#define TWAM03 4
#define TWAM04 5
#define TWAM05 6
#define TWAM06 7

/*-------  TWI1  --------*/
#define TWBR1 _SFR_MEM8(0xD8)
#define TWBR10 0
#define TWBR11 1
#define TWBR12 2
#define TWBR13 3
#define TWBR14 4
#define TWBR15 5
#define TWBR16 6
#define TWBR17 7

#define TWSR1 _SFR_MEM8(0xD9)
#define TWPS10 0
#define TWPS11 1
#define TWS13 3
#define TWS14 4
#define TWS15 5
#define TWS16 6
#define TWS17 7

#define TWAR1 _SFR_MEM8(0xDA)
#define TWGCE1 0
#define TWA10 1
#define TWA11 2
#define TWA12 3
#define TWA13 4
#define TWA14 5
#define TWA15 6
#define TWA16 7

#define TWDR1 _SFR_MEM8(0xDB)
#define TWD10 0
#define TWD11 1
#define TWD12 2
#define TWD13 3
#define TWD14 4
#define TWD15 5
#define TWD16 6
#define TWD17 7

#define TWCR1 _SFR_MEM8(0xDC)
#define TWIE1 0
#define TWEN1 2
#define TWWC1 3
#define TWSTO1 4
#define TWSTA1 5
#define TWEA1 6
#define TWINT1 7

#define TWAMR1 _SFR_MEM8(0xDD)
#define TWAM10 1
#define TWAM11 2
#define TWAM12 3
#define TWAM13 4
#define TWAM14 5
#define TWAM15 6
#define TWAM16 7

#endif