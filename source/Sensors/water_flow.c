#include <avr/interrupt.h>
#include "water_flow.h"

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// liter/minute of flow.
const float calibrationFactor = 4.5;
static volatile uint16_t pulseCount = 0;
unsigned int flowMilliLitres;

/*Updates flow rate, and total milliliters */
void check_flow_sensor(unsigned long current_time, unsigned long previous_time, float *flow_rate, unsigned long *total_milliLitres)
{
	// Because this loop may not complete in exactly 1 second intervals we calculate
	// the number of milliseconds that have passed since the last execution and use
	// that to scale the output. We also apply the calibrationFactor to scale the output
	// based on the number of pulses per second per units of measure (liters/minute in
	// this case) coming from the sensor.
	*flow_rate = ((1000.0 / (current_time - previous_time)) * pulseCount) / calibrationFactor;  // L/m

	// Divide the flow rate in liters/minute by 60 to determine how many liters have
	// passed through the sensor in this 1 second interval, then multiply by 1000 to
	// convert to milliliters.
	flowMilliLitres = (*flow_rate / 60) * 1000;   // mL/sec

	// Add the milliliters passed in this second to the cumulative total
	*total_milliLitres += flowMilliLitres;        // mL

	// Reset the pulse counter so we can start incrementing again
	pulseCount = 0;
}

void reset_water_flow_parameters(float *flow_rate, unsigned long *total_milliLitres)
{
	pulseCount          = 0;
	flowMilliLitres     = 0;
	*flow_rate          = 0.0;
	*total_milliLitres  = 0;
}

void setup_water_flow_interrupt(void)
{
	EICRA = 0x02;   // Falling edge
	EIMSK = 0x01;   // Enable EXT0 interrupt.
}

/*Counts the pulses from the water flow sensor attached at interrupt pin 0*/
ISR(INT0_vect)
{
	pulseCount++;
}

