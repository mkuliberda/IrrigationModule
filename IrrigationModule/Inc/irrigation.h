/*
 * Irrigation.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef IRRIGATION_H_
#define IRRIGATION_H_

#include "stm32f3xx_hal.h"
#include <string>
#include "main.h"
#include "gpio.h"

#define MOUNTPOSITION_SENSOR1 	90
#define MOUNTPOSITION_SENSOR2 	10
#define SENSORSAMOUNT_TANK1		2

struct gpio_t{
	GPIO_TypeDef* port;
	uint16_t pin;
};


using namespace std;

class GenericPump{

private:

	enum state_t{
		init 		= (uint8_t) 0,
		running 	= (uint8_t) 1,
		stopped 	= (uint8_t) 2,
		waiting		= (uint8_t) 3
	};


	double runtimeSeconds;
	double idletimeSeconds;
	const uint32_t runtimeLimitSeconds;
	const uint32_t idletimeRequiredSeconds;
	state_t state;
	string description;
	gpio_t pinout;

	void 	stateSet(state_t st);
	void 	runtimeReset(void);
	void 	runtimeIncrease(double dt);
	double 	runtimeGetSeconds(void);
	void 	idletimeReset(void);
	void 	idletimeIncrease(double dt);
	double 	idletimeGetSeconds(void);


public:

	GenericPump(string description, const uint32_t idletimeRequiredSeconds, const uint32_t runtimeLimitSeconds, GPIO_TypeDef* GPIOx, uint16_t pin):
	runtimeSeconds(0.0),
	idletimeSeconds(0.0),
	runtimeLimitSeconds(runtimeLimitSeconds),
	idletimeRequiredSeconds(idletimeRequiredSeconds),
	state(state_t::init),
	description(description)
	{
		this->pinout.pin = pin;
		this->pinout.port = GPIOx;
		this->stop();
	};

	~GenericPump();

	void start(void);
	void stop(void);
	state_t stateGet(void);
	bool isRunning(void);
	string descriptionGet(void);
	void step(double dt);

};

class OpticalWaterLevelSensor{

private:

	enum state_t{
		unknown,
		wet,
		dry
	};

	const uint8_t mount_position;
	state_t state;

public:

	OpticalWaterLevelSensor(const uint8_t position_percent):
		mount_position(position_percent),
		state(state_t::unknown)
		{};

	~OpticalWaterLevelSensor();

	void stateSet(state_t state);
	const uint8_t mountpositionGet(void);
	bool isValid(void);
	bool isWet(void);

};


class WaterTank{

private:

	enum state_t{
		unknown = (uint8_t) 0,
		liquid 	= (uint8_t) 1,
		frozen 	= (uint8_t) 2
	};

	double temperature;
	uint8_t waterlevel;
	bool _isOK;
	state_t state;
	const uint8_t levels_amount;

	void stateSet(state_t state);

	OpticalWaterLevelSensor *sensorTop = new OpticalWaterLevelSensor(MOUNTPOSITION_SENSOR1);
	OpticalWaterLevelSensor *sensorBottom = new OpticalWaterLevelSensor(MOUNTPOSITION_SENSOR2);


public:

	WaterTank(const uint8_t levels_amount):
		temperature(0.0),
		waterlevel(0),
		_isOK(false),
		state(state_t::unknown),
		levels_amount(levels_amount)
	{};

	~WaterTank()
	{
		delete sensorTop;
		delete sensorBottom;
	}

	double temperatureGet(void);
	bool waterlevelSet(uint8_t waterlevel);
	uint8_t waterlevelGet(void);
	bool checkStateOK(void);
	state_t stateGet(void);
	const uint8_t waterlevelsAmountGet(void);

};



class GenericMoistureSensor{

public:

	GenericMoistureSensor():
	moisture_reading_raw(0)
	{};

	~GenericMoistureSensor();

	virtual uint8_t moistureCalculatePercent(void);
	template <typename T> void rawreadingSet(T _moisture_reading_raw);

	uint32_t moisture_reading_raw;
};



class AnalogMoistureSensor: GenericMoistureSensor{

public:

	AnalogMoistureSensor():
	moisture_percent(0)
	{};

	~AnalogMoistureSensor();

	uint8_t moistureCalculatePercent(void);

private:
	uint8_t moisture_percent;
};


//class Controller{
//
//	Controller();
//
//	~Controller()
//	{
//		delete pump1;
//		delete pump2;
//		delete pump3;
//		delete tank1sensorTop;
//		delete tank1sensorBottom;
//		delete tank1;
//	};
//
//public:
//
//	void run(void);
//
//private:
//
//	Pump *pump1 = new Pump("Kroton i Monstera", 2, 10);
//	Pump *pump2 = new Pump("Palma", 2, 10);
//	Pump *pump3 = new Pump("Bluszcz, pelargonie i zonkile", 2, 15);
//	waterlevelSensor *tank1sensorTop = new waterlevelSensor(MOUNTPOSITION_SENSOR1);
//	waterlevelSensor *tank1sensorBottom = new waterlevelSensor(MOUNTPOSITION_SENSOR2);
//	Tank *tank1 = new Tank(SENSORSAMOUNT_TANK1);
//
//};



#endif /* IRRIGATION_H_ */
