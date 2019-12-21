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
#include <vector>


struct gpio_s{
	GPIO_TypeDef* port;
	uint16_t pin;
};

enum pumpState_t: uint8_t{
	init,
	running,
	stopped,
	waiting
};

enum fixedwaterlevelsensorState_t: uint8_t{
	undetermined,
	wet,
	dry
};

enum waterlevelsensorsubtype_t: uint8_t {
	unknown,
	fixed,
	floating
};


using namespace std;

class Pump{

protected:

	pumpState_t state = pumpState_t::init;									///< current pump's working state based on enum pumpState_t

	virtual bool start() = 0;
	virtual bool stop() = 0;

public:

	Pump(){};

	virtual ~Pump(){};

	bool init();
	virtual void run(const double & _dt);
	virtual void stateSet(const pumpState_t & _st) = 0;
	virtual pumpState_t stateGet(void) = 0;
	virtual bool isRunning(void);

};

class BinaryPump: public Pump{

private:

	double runtimeSeconds;					///< current runtime, incrementing in running state [seconds]
	double idletimeSeconds;					///< current idletime incrementing in stopped and waiting state [seconds]
	uint32_t runtimeLimitSeconds;			///< runtime limit for particular pump [seconds]
	uint32_t idletimeRequiredSeconds; 		///< idletime required between two consecutive runs [seconds]
	struct gpio_s pinout;
	struct gpio_s led;
	bool forced = false;

	void 	runtimeReset(void);
	void 	runtimeIncrease(const double & _dt);
	double 	runtimeGetSeconds(void);
	void 	idletimeReset(void);
	void 	idletimeIncrease(const double & _dt);
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


	bool 	init(const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led);
	void 	run(const double & dt, const bool & _cmd_start, bool & cmd_consumed);
	void 	stateSet(const pumpState_t & _st) override;
	pumpState_t stateGet(void) override;
	void forcestart(void);
	void forcestop(void);

};



class WaterLevelSensor{

protected:

	waterlevelsensorsubtype_t subtype;

	waterlevelsensorsubtype_t subtypeGet(void);

public:

	WaterLevelSensor(){};

	virtual ~WaterLevelSensor(){};

	virtual bool init(const waterlevelsensorsubtype_t & _subtype);
};


class OpticalWaterLevelSensor: private WaterLevelSensor{

private:

	const float mountpositionMeters;
	fixedwaterlevelsensorState_t state;
	struct gpio_s pinout;

	const float mountpositionGet(void);
	void read(void);

public:

	OpticalWaterLevelSensor(const float & _mountpositionMeters):
		mountpositionMeters(_mountpositionMeters),
		state(fixedwaterlevelsensorState_t::undetermined)
		{};

	bool init(const waterlevelsensorsubtype_t & _subtype, const struct gpio_s & _pinout);
	bool isValid(void);
	bool isWet(void);

	friend class WaterTank;

};



class WaterTank{

private:

	enum class contentstate_t: uint8_t{
		unknown = 0,
		liquid 	= 1,
		frozen 	= 2
	};

	enum class contentlevel_t: uint8_t{
		unknown		= 255,
		empty		= 0,
		low			= 25,
		medium		= 50,
		high		= 75,
		full 		= 100
	};

	double temperature;
	contentlevel_t waterlevel;
	bool _isOK;
	contentstate_t waterstate;
	const double tankheightMeters;
	const double tankvolumeLiters;

	double temperatureGet(void);
	bool waterlevelSet(const contentlevel_t & _waterlevel);
	contentlevel_t waterlevelGet(void);
	void stateSet(const contentstate_t & _waterstate);
	contentstate_t stateGet(void);
	float waterlevelConvertToPercent(const float & _valMeters);

public:

	WaterTank(const double & _tankheightMeters, const double & _tankvolumeLiters):
		temperature(0.0),
		waterlevel(contentlevel_t::unknown),
		_isOK(false),
		waterstate(contentstate_t::unknown),
		tankheightMeters(_tankheightMeters),
		tankvolumeLiters(_tankvolumeLiters)
	{};

	~WaterTank()
	{};

	vector <OpticalWaterLevelSensor> vWLSensors;

	bool init(const array<float,10> & _arr);
	bool checkStateOK(uint32_t & errcodeBitmask);

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
