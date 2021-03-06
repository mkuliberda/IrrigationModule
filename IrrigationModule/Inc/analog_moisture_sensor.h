/*
 * adc_moisture_sensor.h
 *
 *  Created on: 21.01.2020
 *      Author: Mati
 */

#ifndef ANALOG_MOISTURE_SENSOR_H_
#define ANALOG_MOISTURE_SENSOR_H_


#include "sensors_base.h"
#include "stm32f3xx_hal.h"
#include "gpio.h"
#include "main.h"


class AnalogDMAMoistureSensor: public MoistureSensor{

private:

	uint16_t						moistureRaw;
	float							moistureVolts;

	void		 					percentUpdate(void);
	void 							voltsUpdate(void);

public:

	AnalogDMAMoistureSensor():
	moistureRaw(0),
	moistureVolts(0)
	{
		this->interfacetype = sensorinterfacetype_t::analog;
	};

	~AnalogDMAMoistureSensor(){};

	float		 					read(void);
	bool&							isValid(void);
	void							rawUpdate(const uint16_t & _raw_value);
	float&							voltsGet(void);

};




#endif /* ANALOG_MOISTURE_SENSOR_H_ */
