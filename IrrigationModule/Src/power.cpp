/*
 * battery_manager.cpp
 *
 *  Created on: 18.04.2020
 *      Author: Mati
 */

#include <power.h>


bool& Battery::	isValid(void){
	if (this->voltage_current <= this->voltage_max && this->voltage_current >= this->voltage_min) return this->valid = true;
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
	return this->voltage_current;
}
float& Battery::getCurrent(void){
	return this->current;
}
float Battery::getRemainingCapacity(void){
	if (this->capacity_full > 0) return this->capacity_full * this->getPercentage()/100.0f;
	else return this->capacity_full;
}
float& Battery::getPercentage(void){
	if (this->voltage_current < this->voltage_min) return this->percentage = 0;
	else if (this->voltage_current > this->voltage_max) return this->percentage = 100;
	else return this->percentage = (this->voltage_current - this->voltage_min) / (this->voltage_max - this->voltage_min) * 100.0;
}
const uint8_t& Battery::getCellCount(void){
	return this->cell_count;
}
uint32_t& Battery::getRemainingTimeSeconds(void){
	return this->remaining_time_s;
}
batterystate_t& Battery::getState(void){
	return this->state;
}
batteryerror_t&	Battery::getErrors(void){
	return this->errors;
}
void Battery::configureAdcCharacteristics(const float &_adc_voltage_divider_error_factor, const float &_adc_reference_voltage, const uint32_t &_adc_voltage_levels){
	this->adc_voltage_divider_error_factor = _adc_voltage_divider_error_factor;
	this->adc_reference_voltage = _adc_reference_voltage;
	this->adc_voltage_levels = _adc_voltage_levels;
}

bool Battery::update(const float & _dt){
	return false; //TODO:
}
bool Battery::update(const float & _dt, uint16_t *_raw_adc_values, const uint8_t &_adc_values_count){
	if (this->interface == batteryinterface_t::adc){
		if (this->cell_count == _adc_values_count){
			return true; //TODO
		}
		else return false;
	}
	else return false;
}






