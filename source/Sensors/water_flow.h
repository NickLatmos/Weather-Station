#ifndef WATER_FLOW_H
#define WATER_FLOW_H
#endif

// Water flow sensing pin
#define WATER_FLOW_SENSE_PIN				PD2
#define WATER_FLOW_SENSOR_PORT			PORTD
#define WATER_FLOW_SENSOR_DDR				DDRD

#ifdef __cplusplus
extern "C" {
	void check_flow_sensor(unsigned long current_time, unsigned long previous_time, float *flow_rate, unsigned long *total_milliLitres);
	void reset_water_flow_parameters(float *flow_rate, unsigned long *total_milliLitres);
	void setup_water_flow_interrupt(void);
}
#else
	void check_flow_sensor(unsigned long current_time, unsigned long previous_time, float *flow_rate, unsigned long *total_milliLitres);
	void reset_water_flow_parameters(float *flow_rate, unsigned long *total_milliLitres);
	void setup_water_flow_interrupt(void);
#endif