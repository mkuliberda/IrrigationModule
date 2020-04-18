/*
 * battery_manager.h
 *
 *  Created on: 18.04.2020
 *      Author: Mati
 */

#ifndef POWER_H_
#define POWER_H_

#include "stm32f3xx_hal.h"
#include <utilities.h>

enum class batterytype_t: uint8_t{
	undefined,
	lipo,
	lipo_hv,
	liion,
	liion_hv,
	nimh
};

enum class batterystate_t: uint8_t{
	undetermined,
	charging,
	discharging
};

enum batteryerror_t: uint8_t{
	battery_ok 		= 0,
	overvoltage 	= 2,
	overtemperature = 4,
	overloaded 		= 8,
	max				= 128
};

enum class batteryinterface_t: uint8_t{
	undefined,
	adc,
	i2c,
	can,
	spi
};

struct battery_s{
	uint16_t remaining_time_min;
	uint8_t id;
	uint8_t percentage;
	batterystate_t state;
	batteryerror_t error;
};


class Battery{
private:
	const uint8_t id;
	float voltage_min;
	float voltage_max;
	float voltage_current;
	float voltage_previous;
	float current;
	float percentage;
	bool valid = false;
	const batterytype_t type;
	batterystate_t state;
	batteryerror_t errors;
	uint32_t remaining_time_s;
	const batteryinterface_t interface;
	const uint8_t  cell_count;
	const float capacity_full;
	float adc_voltage_divider_error_factor = 1.0;
	float adc_reference_voltage = 3.0;
	uint32_t adc_voltage_levels = 4095;

public:
	Battery(const uint8_t &_id, const batterytype_t & _type, const batteryinterface_t &_interface, const uint8_t & _cell_count):
	id(_id),
	type(_type),
	interface(_interface),
	cell_count(_cell_count),
	capacity_full(-1000)
	{
		switch(_type){
		case batterytype_t::lipo:
			this->voltage_max = this->cell_count * 4.2;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::lipo_hv:
			this->voltage_max = this->cell_count * 4.35;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::liion:
			this->voltage_max = this->cell_count * 4.2;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::liion_hv:
			this->voltage_max = this->cell_count * 4.35;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::nimh:
			this->voltage_max = this->cell_count * 1.5;
			this->voltage_min = this->cell_count * 1.0;
			break;
		default:
			this->valid = false;
			break;
		}
	}
	Battery(const uint8_t &_id, const batterytype_t & _type, const batteryinterface_t &_interface, const uint8_t & _cell_count, const float & _capacity_full):
	id(_id),
	type(_type),
	interface(_interface),
	cell_count(_cell_count),
	capacity_full(_capacity_full)
	{
		switch(_type){
		case batterytype_t::lipo:
			this->voltage_max = this->cell_count * 4.2;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::lipo_hv:
			this->voltage_max = this->cell_count * 4.35;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::liion:
			this->voltage_max = this->cell_count * 4.2;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::liion_hv:
			this->voltage_max = this->cell_count * 4.35;
			this->voltage_min = this->cell_count * 3.0;
			break;
		case batterytype_t::nimh:
			this->voltage_max = this->cell_count * 1.5;
			this->voltage_min = this->cell_count * 1.0;
			break;
		default:
			this->valid = false;
			break;
		}
	}
	~Battery(){}

	bool& 					isValid(void);
	bool					isCharging(void);
	const uint8_t& 			getId(void);
	const batterytype_t&	getType(void);
	float& 					getVoltage(void);
	float&					getCurrent(void);
	float 					getRemainingCapacity(void);
	float&					getPercentage(void);
	const uint8_t&			getCellCount(void);
	uint32_t&				getRemainingTimeSeconds(void);
	batterystate_t&			getState(void);
	batteryerror_t&			getErrors(void);
	void					configureAdcCharacteristics(const float &_adc_voltage_divider_error_factor, const float &_adc_reference_voltage, const uint32_t &_adc_voltage_levels);
	bool					update(const float & _dt);
	bool					update(const float & _dt, uint16_t *_raw_adc_values, const uint8_t &_adc_values_count);
};



#endif /* POWER_H_ */
