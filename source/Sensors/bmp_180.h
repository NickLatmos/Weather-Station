#ifndef BMP_180_H
#define BMP_180_H

#include <stdbool.h>
#include <stdint.h>

#define BMP180_DEBUG 0

#define BMP180_I2CADDR 0x77
#define SHIFTED_BMP180_I2CADDRESS (BMP180_I2CADDR<<1)   //First 7 bits contain the address and the last R/W

#define BMP180_ULTRALOWPOWER 0
#define BMP180_STANDARD      1
#define BMP180_HIGHRES       2
#define BMP180_ULTRAHIGHRES  3
#define BMP180_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP180_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP180_CAL_AC3           0xAE  // R   Calibration data (16 bits)    
#define BMP180_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP180_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP180_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP180_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP180_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP180_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP180_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP180_CAL_MD            0xBE  // R   Calibration data (16 bits)

#define BMP180_CONTROL           0xF4  // Control register
#define BMP180_TEMPDATA          0xF6  // Raw temperature data in registers 0xF6 (MSB), 0xF7 (LSB)
#define BMP180_PRESSUREDATA      0xF6  // Pressure in registers 0xF6 (MSB), 0xF7 (LSB), 0xF8 (xLSB)
#define BMP180_READTEMPCMD       0x2E  // Read temperature (4.5ms)
#define BMP180_READPRESSURECMD   0x34


class BMP180 {
 public:
  BMP180();
  bool begin(uint8_t mode = BMP180_ULTRAHIGHRES);  // by default go high resolution
  float readTemperature(void);
  int32_t readPressure(void);
  int32_t readSealevelPressure(float altitude_meters = 0);
  float readAltitude(float sealevelPressure = 101325); // std atmosphere
  uint16_t readRawTemperature(void);
  uint32_t readRawPressure(void);
  
 private:
  int32_t computeB5(int32_t UT);
  uint8_t read8(uint8_t addr);
  uint16_t read16(uint8_t addr);
  void write8(uint8_t addr, uint8_t data);

  uint8_t oversampling;

  int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
  uint16_t ac4, ac5, ac6;
};


#endif