#ifndef ADC_H
#define ADC_H

#ifdef __cplusplus
  extern "C" {
		void adc_init(void);
		uint16_t adc_read(uint8_t ch);	
		void adc_shutdown(void);
	}
#else 
		void adc_init(void);
		uint16_t adc_read(uint8_t ch);
		void adc_shutdown(void);
#endif

#endif