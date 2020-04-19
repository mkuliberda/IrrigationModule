/*
 * battery_manager.cpp
 *
 *  Created on: 18.04.2020
 *      Author: Mati
 */

#include <power.h>


bool& Battery::	isValid(void){
	if (this->voltage <= (this->cell_voltage_max * this->cell_count) && \
			this->voltage >= (this->cell_voltage_min * this->cell_count) &&\
			this->errors == batteryerror_t::battery_ok) return this->valid = true;
	else return this->valid = false;
}
bool Battery::isCharging(void){
	return this->state == batterystate_t::charging ? true : false;
}
const uint8_t& Battery::getId(void){
	return this->id;
}
const batterytype_t& Battery::getType(void){
	return this->type;
}
float& Battery::getVoltage(void){
	return this->voltage;
}
float& Battery::getCurrent(void){
	return this->current;
}
float Battery::getRemainingCapacity(void){
	if (this->capacity_full > 0) return this->capacity_full * this->getPercentage()/100.0f;
	else return this->capacity_full;
}
float& Battery::getPercentage(void){
	return this->percentage;
}
const uint8_t& Battery::getCellCount(void){
	return this->cell_count;
}
uint16_t& Battery::getRemainingTimeMinutes(void){
	return this->remaining_time_min;
}
batterystate_t& Battery::getState(void){
	return this->state;
}
batteryerror_t&	Battery::getErrors(void){
	return this->errors;
}
void Battery::configureAdcCharacteristics(const float &_adc_voltage_divider_error_factor, const float &_adc_reference_voltage, const uint32_t &_adc_levels){
	this->adc_voltage_divider_error_factor = _adc_voltage_divider_error_factor;
	this->adc_reference_voltage = _adc_reference_voltage;
	this->adc_levels = _adc_levels;
}

void Battery::calculatePercentage(void){
	if (this->voltage < (this->cell_voltage_min * this->cell_count)) this->percentage = 0;
	else if (this->voltage > (this->cell_voltage_max * this->cell_count)) this->percentage = 100;
	else this->percentage = (this->voltage - (this->cell_voltage_min * this->cell_count)) / ((this->cell_voltage_max * this->cell_count) - (this->cell_voltage_min * this->cell_count)) * 100.0;
}

void Battery::calculateRemainingTimeMinutes(const float &_dt){
	//TODO
}

void Battery::determineState(const float &_dt){
	if((this->dt_from_last_calc += _dt) > this->calc_timespan_s){
		this->voltage > this->voltage_previous ? this->state = batterystate_t::charging : this->state = batterystate_t::discharging;
		this->voltage_previous = this->voltage;
		this->dt_from_last_calc = 0;
	}
}

bool Battery::update(const float & _dt){
	return false; //TODO:
}
bool Battery::update(const float & _dt, uint16_t *_raw_adc_values, const uint8_t &_adc_values_count){
	bool success = true;

	if (this->interface == batteryinterface_t::adc){
		if (this->cell_count == _adc_values_count){
			this->voltage = 0;
			for (uint8_t i=0; i<this->cell_count; ++i){
				float cell_voltage = (_raw_adc_values[i] * this->adc_reference_voltage / this->adc_levels) * this->adc_voltage_divider_error_factor;
				this->voltage += cell_voltage;
				this->calculatePercentage();
				this->determineState(_dt);
			}
		}
		else if (this->cell_count > 1 && _adc_values_count == 1){
			this->voltage = (_raw_adc_values[0] * this->adc_reference_voltage / this->adc_levels) * this->adc_voltage_divider_error_factor;
			this->calculatePercentage();
			this->determineState(_dt);
		}
		else success = false;
		//TODO: errors set/reset
	}
	else{
		success = false;
	}

	return success;
}






