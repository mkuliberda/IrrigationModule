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
#include "tim.h"
#include <array>
#include <vector>
#include <memory>
#include <utilities.h>
#include <bitset>
#include <numeric>


struct gpio_s{
	GPIO_TypeDef* port;
	uint16_t pin;
};

struct pumpstatus_s {
	uint8_t id = 0;
	uint32_t state = 0;
	bool forced = false;
	bool cmd_consumed = false;
};

enum class pumpstate_t{
	init,
	running,
	reversing,
	stopped,
	waiting
};

enum class pumptype_t{
	generic,
	binary,
	drv8833_dc,
	drv8833_bldc
};

enum class motortype_t{
	dc_motor,
	bldc_motor
};

enum class pumpcmd_t{
	start,
	stop,
	reverse
};

enum class fixedwaterlevelsensorstate_t{
	undetermined,
	wet,
	dry
};

enum class waterlevelsensorsubtype_t{
	unknown,
	fixed,
	floating
};

enum class waterlevelsensortype_t{
	unknown,
	optical,
	capacitive,
	resistive
};

enum class sensorinterfacetype_t {
	gpio,
	analog,
	digital_I2C,
	digital_SPI,
	digital_1Wire,
	digital_UART,
	digital_CAN
};

enum class temperaturesensortype_t{
	generic,
	ds18b20
};

enum class moisturesensortype_t{
	generic,
	capacitive_noshield
};

enum class pumpcontrollermode_t{
	init,
	automatic,
	manual,
	external,
	sleep
};

using namespace std;

class Pump{

protected:

	pumptype_t					type = pumptype_t::generic;
	pumpstate_t 				state = pumpstate_t::init;		///< current pump's working state based on enum pumpstate_t
	struct pumpstatus_s 		status;


	virtual bool 				start() = 0;
	virtual bool 				stop() = 0;

public:

	Pump(){};

	virtual ~Pump(){};

	bool init();
	virtual void 				run(const double & _dt);
	virtual void 				stateSet(const pumpstate_t & _st) = 0;
	virtual pumpstate_t& 		stateGet(void) = 0;
	virtual bool 				isRunning(void);
	struct pumpstatus_s&		statusGet(void);

};

class BinaryPump: public Pump{

private:

	double 						runtimeSeconds;					///< current runtime, incrementing in running state [seconds]
	double 						idletimeSeconds;					///< current idletime incrementing in stopped and waiting state [seconds]
	uint32_t 					runtimeLimitSeconds;			///< runtime limit for particular pump [seconds]
	uint32_t 					idletimeRequiredSeconds; 		///< idletime required between two consecutive runs [seconds]
	struct gpio_s 				pinout;
	struct gpio_s 				led;

	void 						runtimeReset(void);
	void 						runtimeIncrease(const double & _dt);
	double& 					runtimeGetSeconds(void);
	void 						idletimeReset(void);
	void 						idletimeIncrease(const double & _dt);
	double& 					idletimeGetSeconds(void);

protected:

	bool 						start(void) override;
	bool 						stop(void) override;

public:

	BinaryPump():
	runtimeSeconds(0.0),
	idletimeSeconds(0.0),
	runtimeLimitSeconds(0),
	idletimeRequiredSeconds(0)
	{
		this->type=pumptype_t::binary;
	};

	bool 						init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led);
	void 						run(const double & _dt, const bool & _cmd_start, bool & cmd_consumed);
	void 						stateSet(const pumpstate_t & _st) override;
	pumpstate_t& 				stateGet(void) override;
	void 						forcestart(void);
	void 						forcestop(void);

};

class DRV8833Pump: public Pump{

private:

	double 						runtimeSeconds;					///< current runtime, incrementing in running state [seconds]
	double						revtimeSeconds;
	double 						idletimeSeconds;				///< current idletime incrementing in stopped and waiting state [seconds]
	uint32_t 					runtimeLimitSeconds;			///< runtime limit for particular pump [seconds]
	uint32_t					revtimeLimitSeconds;
	uint32_t 					idletimeRequiredSeconds; 		///< idletime required between two consecutive runs [seconds]
	array<struct gpio_s, 4> 	aIN;							///< in1, in2, in3, in4
	struct gpio_s 				led;
	struct gpio_s 				fault;
	struct gpio_s 				mode;

	void 						runtimeReset(void);
	void 						runtimeIncrease(const double & _dt);
	double&						runtimeGetSeconds(void);
	void 						idletimeReset(void);
	void 						idletimeIncrease(const double & _dt);
	double& 					idletimeGetSeconds(void);
	void 						revtimeReset(void);
	void 						revtimeIncrease(const double & _dt);
	double& 					revtimeGetSeconds(void);


protected:

	bool 						start(void) override;
	bool 						stop(void) override;
	bool						reverse(void);


public:

	DRV8833Pump(const motortype_t & _type):
	runtimeSeconds(0.0),
	idletimeSeconds(0.0),
	runtimeLimitSeconds(0),
	idletimeRequiredSeconds(0)
	{
		if (_type == motortype_t::bldc_motor) this->type = pumptype_t::drv8833_bldc;
		if (_type == motortype_t::dc_motor) this->type = pumptype_t::drv8833_dc;
	};

	bool 						init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, \
								const array<struct gpio_s, 4> & _pinout, const struct gpio_s & _led, \
								const struct gpio_s & _fault, const struct gpio_s & _mode);
	bool 						init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, \
								const array<struct gpio_s, 2> & _pinout, const struct gpio_s & _led, \
								const struct gpio_s & _fault, const struct gpio_s & _mode);
	void 						run(const double & _dt, const pumpcmd_t & _cmd, bool & cmd_consumed);
	void 						stateSet(const pumpstate_t & _st) override;
	pumpstate_t& 				stateGet(void) override;
	void 						forcestart(void);
	void 						forcereverse(void);
	void 						forcestop(void);
	void 						sleepmodeSet(void);
	bool						faultCheck(void);

};

class MoistureSensor{

protected:

	moisturesensortype_t 			type;
	sensorinterfacetype_t			interfacetype;
	float 							moisturePercent;
	bool							valid;

	moisturesensortype_t& 			typeGet(void);
	sensorinterfacetype_t& 			interfacetypeGet(void);
	virtual void 					percentUpdate(void) = 0;

public:

	MoistureSensor():
	moisturePercent(0),
	valid(false)
	{
		this->type = moisturesensortype_t::generic;
	};

	virtual ~MoistureSensor(){};

	virtual float		 			read(void) = 0;
	virtual bool&					isValid(void) = 0;
	float&							percentGet(void);

};


class AnalogDMAMoistureSensor: MoistureSensor{

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

class WaterLevelSensor{

protected:

	waterlevelsensortype_t 				type;
	waterlevelsensorsubtype_t 			subtype;
	sensorinterfacetype_t				interfacetype;

	virtual waterlevelsensorsubtype_t& 	subtypeGet(void);
	virtual waterlevelsensortype_t&		typeGet(void);
	virtual sensorinterfacetype_t& 		interfacetypeGet(void);

public:

	WaterLevelSensor(){};

	virtual ~WaterLevelSensor(){};

};


class OpticalWaterLevelSensor: public WaterLevelSensor{

private:

	float 							mountpositionMeters;
	fixedwaterlevelsensorstate_t 	state;
	struct gpio_s 					pinout;

	void 							read(void);

public:

	OpticalWaterLevelSensor():
		mountpositionMeters(0),
		state(fixedwaterlevelsensorstate_t::undetermined)
		{
			this->type = waterlevelsensortype_t::optical;
			this->subtype = waterlevelsensorsubtype_t::fixed;
			this->interfacetype = sensorinterfacetype_t::gpio;
		};

	const float& 					mountpositionGet(void);
	bool 							init(const float & _mountpositionMeters, const struct gpio_s & _pinout);
	bool 							isValid(void);
	bool 							isSubmersed(void);

};

class TemperatureSensor{

protected:

	temperaturesensortype_t 		type;
	sensorinterfacetype_t			interfacetype;

	temperaturesensortype_t& 		typeGet(void);
	sensorinterfacetype_t& 			interfacetypeGet(void);

public:

	TemperatureSensor(){};

	virtual ~TemperatureSensor(){};

};

class DS18B20: public TemperatureSensor{

private:

	bool 							valid;
	struct gpio_s 					gpio;
	TIM_HandleTypeDef* 				timer;

	bool 							prep(void);
	void 							delay_us (const uint32_t & _us);
	void 							gpioSetInput (void);
	void 							gpioSetOutput (void);
	void 							write (const uint8_t & _data);
	uint8_t 						read(void);

public:

	DS18B20():
		valid(false)
	{
		this->interfacetype = sensorinterfacetype_t::digital_1Wire;
		this->type = temperaturesensortype_t::ds18b20;
	};

	bool 							init(const struct gpio_s & _gpio, TIM_HandleTypeDef* _tim_baseHandle);
	bool& 							isValid(void);
	float 							temperatureCelsiusRead(void);

};

class WaterTank{

private:

	enum class contentstate_t: uint8_t{
		unknown 	= 255,
		liquid 		= 1,
		frozen 		= 2,
		boiling		= 3
	};

	enum class contentlevel_t: uint8_t{
		unknown		= 255,
		empty		= 0,
		above10		= 10,
		above20		= 20,
		above30		= 30,
		above40		= 40,
		above50		= 50,
		above60		= 60,
		above70		= 70,
		above80		= 80,
		above90		= 90,
		full 		= 100
	};

	float								mean_watertemperatureCelsius;
	contentlevel_t						waterlevel;
	contentstate_t 						waterstate;
	const double 						tankheightMeters;
	const double 						tankvolumeLiters;
	const int8_t 						waterlevelSensorsLimit;
	int8_t								waterlevelSensorsCount;
	const int8_t 						temperatureSensorsLimit;
	int8_t								temperatureSensorsCount;

	void 								waterlevelSet(const contentlevel_t & _waterlevel);
	contentlevel_t&						waterlevelGet(void);
	void 								stateSet(const contentstate_t & _waterstate);
	contentstate_t&						stateGet(void);
	uint8_t 							waterlevelConvertToPercent(const float & _valMeters);

public:

	WaterTank(const double & _tankheightMeters, const double & _tankvolumeLiters):
		mean_watertemperatureCelsius(0.0),
		waterlevel(contentlevel_t::unknown),
		waterstate(contentstate_t::unknown),
		tankheightMeters(_tankheightMeters),
		tankvolumeLiters(_tankvolumeLiters),
		waterlevelSensorsLimit(10),
		waterlevelSensorsCount(0),
		temperatureSensorsLimit(3),
		temperatureSensorsCount(0)
	{};

	~WaterTank()
	{};

	vector <OpticalWaterLevelSensor> 	vOpticalWLSensors;
	vector <DS18B20> 					vTemperatureSensors;

	bool 								init(void);
	bool 								checkStateOK(uint32_t & errcodeBitmask);
	float& 								temperatureCelsiusGet(void);
	uint8_t		 						waterlevelPercentGet(void);
	bool 								waterlevelSensorCreate(const waterlevelsensortype_t & _sensortype);
	bool 								temperatureSensorCreate(const temperaturesensortype_t & _sensortype);

};

class PumpController{

private:

	const int8_t 						pumpsLimit = 1;
	int8_t								pumpsCount;
	const int8_t						moisturesensorsLimit = 10;
	int8_t								moisturesensorsCount;
	pumpcontrollermode_t				mode;

public:

	PumpController():
		pumpsCount(0),
		moisturesensorsCount(0),
		mode(pumpcontrollermode_t::init)
	{};

	~PumpController()
	{
		if (pBinPump != nullptr) delete pBinPump;
		if (p8833Pump != nullptr) delete p8833Pump;
	};

	BinaryPump							*pBinPump = nullptr;
	DRV8833Pump							*p8833Pump = nullptr;
	vector <AnalogDMAMoistureSensor> 	vDMAMoistureSensor;

	uint8_t								update(const double & _dt, const bool & _activate_watering);
	bool								pumpCreate(const pumptype_t & _pumptype);
	bool 								moisturesensorCreate(const moisturesensortype_t & _sensortype);
	bool								modeSet(const pumpcontrollermode_t & _mode);
	const pumpcontrollermode_t&			modeGet(void);

};


void pumpStateEncode(const struct pumpstatus_s & _pump, uint32_t & status);
void pumpStateDecode(array<struct pumpstatus_s,4> & a_pump, const bitset<32> & _status);




#endif /* IRRIGATION_H_ */
