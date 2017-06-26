#include <avr/io.h>

// ADC right adjusted
void adc_init(void)
{
	PRR0 &= ~(1<<PRADC);					// Turn on the ADC
	
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// Turn off ADV
void adc_shutdown(void)
{
	ADCSRA &= ~(1<<ADEN);				// Disable ADC
	PRR0 |= (1<<PRADC);					// Shut down the ADC
}

// Select the corresponding channel 0~7
uint16_t adc_read(uint8_t ch)
{
	ch &= 0b00000111; 
	ADMUX = (ADMUX & 0xF8)|ch; 
	
	ADCSRA |= (1<<ADSC);										// start single convertion
	
	while(ADCSRA & (1<<ADSC));							// wait for conversion to complete
	
	return (ADC);
}