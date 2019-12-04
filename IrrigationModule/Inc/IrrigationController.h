/*
 * PumpController.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PUMPCONTROLLER_H_
#define PUMPCONTROLLER_H_

#include "stm32f3xx_hal.h"
#include <string>

using namespace std;

class Pump{

private:

	enum state_t{
		init 		= (uint8_t) 0,
		running 	= (uint8_t) 1,
		stopped 	= (uint8_t) 2,
		waiting		= (uint8_t) 3,
	};


	double runtimeSeconds;
	double idletimeSeconds;
	const uint32_t runtimeLimitSeconds;
	const uint32_t idletimeRequiredSeconds;
	state_t state;
	string description;

	void 	stateSet(state_t st);
	void 	runtimeReset(void);
	void 	runtimeIncrease(double dt);
	double 	runtimeGet(void);
	void 	idletimeReset(void);
	void 	idletimeIncrease(double dt);
	double 	idletimeGet(void);


public:
	Pump(string description, const uint32_t idletimeRequiredSeconds, const uint32_t runtimeLimitSeconds):
	runtimeSeconds(0.0),
	idletimeSeconds(0.0),
	runtimeLimitSeconds(runtimeLimitSeconds),
	idletimeRequiredSeconds(idletimeRequiredSeconds),
	state(state_t::init),
	description(description)
	{
		this->stop();
	};

	~Pump();

	void start(void);
	void stop(void);
	state_t stateGet(void);
	bool isRunning(void);
	string descriptionGet(void);
	void step(double dt);

};

class Controller{

	Controller();

	~Controller()
	{
		delete p1;
		delete p2;
		delete p3;
	};

public:

	bool run(void);

private:

	Pump *p1 = new Pump("Kroton i Monstera", 2, 10);
	Pump *p2 = new Pump("Palma", 2, 10);
	Pump *p3 = new Pump("Bluszcz, pelargonie i zonkile", 2, 15);

};

class Tank{

private:

	Tank():
		temperature(0.0),
		waterlevel(0),
		_isOK(false),
		state(state_t::unknown)
		{};

	~Tank();

	enum state_t{
		unknown = (uint8_t) 0,
		liquid 	= (uint8_t) 1,
		frozen 	= (uint8_t) 2
	};


	double temperature;
	uint8_t waterlevel;
	bool _isOK;
	state_t state;

	void stateSet(state_t state);

public:
	bool temperatureSet(double temperature);
	double temperatureGet(void);
	bool waterlevelSet(uint8_t waterlevel);
	uint8_t waterlevelGet(void);
	bool checkStateOK(void);
	state_t stateGet(void);

};




#endif /* PUMPCONTROLLER_H_ */
