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
#include <array>


struct pumpgpio_s{
	GPIO_TypeDef* port;
	uint16_t pin;
};

enum pumpstate_t: uint8_t{
	init,
	running,
	stopped,
	waiting
};

using namespace std;

class Pump{

protected:

	pumpstate_t state = pumpstate_t::init;									///< current pump's working state based on enum pumpstate_t

	virtual bool start() = 0;
	virtual bool stop() = 0;

public:

	Pump(){};

	virtual ~Pump(){};

	bool init();
	virtual void run(double & dt);
	virtual void stateSet(pumpstate_t st) = 0;
	virtual pumpstate_t stateGet(void) = 0;
	virtual bool isRunning(void);

};

class BinaryPump: public Pump{

private:

	double runtimeSeconds;							///< current runtime, incrementing in running state [seconds]
	double idletimeSeconds;							///< current idletime incrementing in stopped and waiting state [seconds]
	uint32_t runtimeLimitSeconds;				///< runtime limit for particular pump [seconds]
	uint32_t idletimeRequiredSeconds; 		///< idletime required between two consecutive runs [seconds]
	struct pumpgpio_s pinout;
	struct pumpgpio_s led;

	void 	runtimeReset(void);
	void 	runtimeIncrease(double & dt);
	double 	runtimeGetSeconds(void);
	void 	idletimeReset(void);
	void 	idletimeIncrease(double & dt);
	double 	idletimeGetSeconds(void);

protected:
	bool 	start(void) override;
	bool 	stop(void) override;


public:

	BinaryPump():
	runtimeSeconds(0.0),
	idletimeSeconds(0.0),
	runtimeLimitSeconds(0),
	idletimeRequiredSeconds(0)
	{};


	bool 	init(uint32_t idletimeRequiredSeconds, uint32_t runtimeLimitSeconds, struct pumpgpio_s _pinout, struct pumpgpio_s _led);
	void 	run(double & dt, bool & cmd_start, bool & cmd_consumed);
	void 	stateSet(pumpstate_t st) override;
	pumpstate_t stateGet(void) override;
	void forcestart(void);
	void forcestop(void);

};



class WaterLevelSensor{

protected:

	enum class subtype_t: uint8_t {
		unknown,
		fixed,
		floating
	};

	subtype_t subtype;

public:

	WaterLevelSensor(){};

	virtual ~WaterLevelSensor(){};

	subtype_t subtypeGet(void);
	void subtypeSet(subtype_t subtype);
	float convertToPercent(float val, float range);


};


class OpticalWaterLevelSensor: public WaterLevelSensor{

private:

	enum state_t{
		unknown,
		wet,
		dry
	};

	const float mountpositionMeters;
	state_t state;

public:

	OpticalWaterLevelSensor(const float mountpositionMeters):
		mountpositionMeters(mountpositionMeters),
		state(state_t::unknown)
		{};

	void stateSet(state_t state);
	const float mountpositionGet(void);
	bool isValid(void);
	bool isWet(void);

};




class WaterTank{

private:

	enum class state_t: uint8_t{
		unknown = 0,
		liquid 	= 1,
		frozen 	= 2
	};

	double temperature;
	uint8_t waterlevel;
	bool _isOK;
	state_t state;
	array<float, 10> levels;
	const double heightMeters;

	void stateSet(state_t state);

public:

	WaterTank(const double heightMeters):
		temperature(0.0),
		waterlevel(0),
		_isOK(false),
		state(state_t::unknown),
		heightMeters(heightMeters)
	{};

	~WaterTank()
	{
		delete WLSensor1;
		delete WLSensor2;
	}


	double temperatureGet(void);
	bool waterlevelSet(uint8_t & waterlevel);
	uint8_t waterlevelGet(void);
	bool checkStateOK(void);
	state_t stateGet(void);

	OpticalWaterLevelSensor *WLSensor1 = new OpticalWaterLevelSensor(0.25);
	OpticalWaterLevelSensor *WLSensor2 = new OpticalWaterLevelSensor(0.08);

};



//class MoistureSensor{
//
//public:
//
//	MoistureSensor():
//	moisture_reading_raw(0)
//	{};
//
//	virtual ~MoistureSensor(){};
//
//	virtual uint8_t moistureCalculatePercent(void);
//	template <typename T> void rawreadingSet(T & _moisture_reading_raw);
//
//	uint32_t moisture_reading_raw;
//};
//
//
//
//class AnalogMoistureSensor: MoistureSensor{
//
//public:
//
//	AnalogMoistureSensor():
//	moisture_percent(0)
//	{};
//
//	~AnalogMoistureSensor(){};
//
//	uint8_t moistureCalculatePercent(void);
//
//private:
//	uint8_t moisture_percent;
//};




#endif /* IRRIGATION_H_ */
