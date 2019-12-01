/*
 * PumpController.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PUMPCONTROLLER_H_
#define PUMPCONTROLLER_H_

#include "stm32f3xx_hal.h"

class Pump{

	Pump(const uint32_t _wait_time, uint32_t _time_limit):
	_running(false),
	runningTimeSeconds(0),
	waitTimeSeconds(_wait_time),
	runningLimitSeconds(_time_limit),
	state(state_t::init)
	{};

	~Pump();

public:
	bool isRunning(void);
	void turnON(void);
	void turnOFF(void);
	uint32_t incrementRunningTime(void);
	void resetRunningTime(void);
	uint32_t getTimeFromLastRun(void);


private:
	bool _running;
	uint32_t runningTimeSeconds;
	uint32_t timeFromLastRunSeconds;
	const uint32_t runningLimitSeconds;
	const uint32_t waitTimeSeconds;
	uint8_t state;

	enum state_t{
		init 		= (uint8_t) 0,
		running 	= (uint8_t) 1,
		stopped 	= (uint8_t) 2,
		force_stop	= (uint8_t) 3
	};

};

class Controller{

};

class Tank{

	Tank():
		temperature(0.0),
		waterLevel(0),
		_isOK(false)
		{};

	~Tank();

private:
	double temperature;
	uint8_t waterLevel;
	bool _isOK;

public:
	bool setTemperature(double temp);
	double getTemperature(void);
	bool setWaterLevel(uint8_t water_lvl);
	uint8_t getWaterLevel(void);
	bool checkStateOK(void);

};


#endif /* PUMPCONTROLLER_H_ */
