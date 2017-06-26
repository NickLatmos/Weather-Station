// Adafruit's modified library for BMP085/BMP180

#include "../processor.h"

#include <util/delay.h>
#include "bmp_180.h"
#include "../Protocols/i2c_master.h"

BMP180::BMP180() {
}

bool BMP180::begin(uint8_t mode) {
  if (mode > BMP180_ULTRAHIGHRES) 
    mode = BMP180_ULTRAHIGHRES;
  oversampling = mode;
	
  i2c0_init();
	
	uint8_t id = 0;
	i2c0_readReg(SHIFTED_BMP180_I2CADDRESS, 0xD0, &id, 1);							//request 1 byte (device id) written in register 0xD0.
	if(id != 0x55) return 0;
	
  /* read calibration data (2 bytes)*/
	uint16_t temp[11];
	i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC1, &temp[0]);
	i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC2, &temp[1]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC3, &temp[2]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC4, &temp[3]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC5, &temp[4]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_AC6, &temp[5]);

  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_B1, &temp[6]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_B2, &temp[7]);

  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_MB, &temp[8]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_MC, &temp[9]);
  i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_CAL_MD, &temp[10]);
	
	ac1 = (int16_t) temp[0];
	ac2 = (int16_t) temp[1];
	ac3 = (int16_t) temp[2];
	ac4 =  temp[3];
	ac5 =  temp[4];
	ac6 =  temp[5];
		
	b1 = (int16_t) temp[6];
	b2 = (int16_t) temp[7];
	
	mb = (int16_t) temp[8];
	mc = (int16_t) temp[9];
	md = (int16_t) temp[10];							
	
  return 1;
}

int32_t BMP180::computeB5(int32_t UT) {
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1+(int32_t)md);
  return X1 + X2;
}

uint16_t BMP180::readRawTemperature(void) {
  //write8(BMP180_CONTROL, BMP180_READTEMPCMD);
	uint16_t raw_temperature;
	uint8_t data = BMP180_READTEMPCMD;
	i2c0_writeReg(SHIFTED_BMP180_I2CADDRESS, BMP180_CONTROL, &data, 1);
  _delay_ms(5);

  //return read16(BMP085_TEMPDATA);
	i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_TEMPDATA, &raw_temperature);
  return raw_temperature;
}

uint32_t BMP180::readRawPressure(void) {
  uint32_t raw;
	uint16_t temp;
	uint8_t temp2;
	
  //write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));
	uint8_t data = BMP180_READPRESSURECMD + (oversampling << 6);
	i2c0_writeReg(SHIFTED_BMP180_I2CADDRESS, BMP180_CONTROL, &data, 1);

  if (oversampling == BMP180_ULTRALOWPOWER) 
    _delay_ms(5);
  else if (oversampling == BMP180_STANDARD) 
    _delay_ms(8);
  else if (oversampling == BMP180_HIGHRES) 
    _delay_ms(14);
  else 
    _delay_ms(26);

  //raw = read16(BMP085_PRESSUREDATA);
	i2c0_read_uint16_register(SHIFTED_BMP180_I2CADDRESS, BMP180_PRESSUREDATA, &temp);
	raw = (uint32_t) temp;
	
  raw <<= 8;
  //raw |= read8(BMP085_PRESSUREDATA+2);
	i2c0_readReg(SHIFTED_BMP180_I2CADDRESS, BMP180_PRESSUREDATA+2, &temp2, 1);
	raw |= temp2;
  raw >>= (8 - oversampling);     // ??

 /* this pull broke stuff, look at it later?
  if (oversampling==0) {
    raw <<= 8;
    raw |= read8(BMP085_PRESSUREDATA+2);
    raw >>= (8 - oversampling);
  }
 */

  return raw;
}


int32_t BMP180::readPressure(void) {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = readRawTemperature();
  UP = readRawPressure();

  B5 = computeB5(UT);

  // do pressure calculationss
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791)>>4);

  return p;
}

int32_t BMP180::readSealevelPressure(float altitude_meters) {
  float pressure = readPressure();
  return (int32_t)(pressure / pow(1.0-altitude_meters/44330, 5.255));
}

float BMP180::readTemperature(void) {
  int32_t UT, B5;     // following ds convention
  float temp;

  UT = readRawTemperature();

  B5 = computeB5(UT);
  temp = (B5+8) >> 4;
  temp /= 10;
  
  return temp;
}

float BMP180::readAltitude(float sealevelPressure) {
  float altitude;

  float pressure = readPressure();

  altitude = 44330 * (1.0 - pow(pressure /sealevelPressure,0.1903));

  return altitude;
}
