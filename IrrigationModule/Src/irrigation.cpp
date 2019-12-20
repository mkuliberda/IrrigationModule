/**
 * Irrigation control file with classes for sensors, tanks, pumps etc
 */

#include <irrigation.h>


/******************************/
/*! Pump class implementation */
/******************************/

bool Pump::init(void){
	return true;
}

bool Pump::isRunning(void){
	return stateGet() == pumpstate_t::running ? true : false;
}

void Pump::run(double & dt){

}


/************************************/
/*! BinaryPump class implementation */
/************************************/

bool BinaryPump::init(const uint32_t idletimeRequiredSeconds, const uint32_t runtimeLimitSeconds, struct pumpgpio_s _pinout, struct pumpgpio_s _led){
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = idletimeRequiredSeconds;
	this->runtimeLimitSeconds = runtimeLimitSeconds;
	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpstate_t::stopped);

	return this->isRunning();
}

void BinaryPump::run(double & dt, bool & cmd_start, bool & cmd_consumed){

	switch (this->stateGet())
	{
	case pumpstate_t::init:
		this->stop();
		cmd_consumed = true;
		break;

	case pumpstate_t::waiting:
		this->idletimeIncrease(dt);
		break;

	case pumpstate_t::stopped:
		this->idletimeIncrease(dt);
		if((cmd_start == true) && (cmd_consumed == false) && (this->idletimeGetSeconds() > this->idletimeRequiredSeconds)){
			if (this->start() == true ) cmd_consumed = true;
		}
		break;

	case pumpstate_t::running:
		this->runtimeIncrease(dt);
		if(cmd_start == true){
			cmd_consumed = true;
		}
		else{
			if(this->stop() == true) cmd_consumed = true;
		}
		if(this->runtimeGetSeconds() > this->runtimeLimitSeconds) this->stop();
		break;

	default:
		break;
	}
}

bool BinaryPump::start(void){

	bool success = false;

	if(this->isRunning() == false){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
		this->stateSet(pumpstate_t::running);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

bool BinaryPump::stop(void){

	bool success = false;

	if(this->isRunning() == true){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
		this->stateSet(pumpstate_t::stopped);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

void BinaryPump::forcestart(void){

	if(this->isRunning() == false) this->runtimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
	this->stateSet(pumpstate_t::running);
}
void BinaryPump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpstate_t::stopped);
}


pumpstate_t BinaryPump::stateGet(void){
	return this->state;
}

void BinaryPump::stateSet(pumpstate_t state){
	this->state = state;
}

void BinaryPump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void BinaryPump::runtimeIncrease(double & dt){
	this->runtimeSeconds += dt;
}

double BinaryPump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void BinaryPump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void BinaryPump::idletimeIncrease(double & dt){
	this->idletimeSeconds += dt;
}

double BinaryPump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}




/******************************************/
/*! WaterLevelSensor class implementation */
/******************************************/
WaterLevelSensor::subtype_t  WaterLevelSensor::subtypeGet(void){
	return this->subtype;
}

void WaterLevelSensor::subtypeSet(subtype_t subtype){
	this->subtype = subtype;
}

float WaterLevelSensor::convertToPercent(float val, float range){
	return val/range*100;
}



/*************************************************/
/*! OpticalWaterLevelSensor class implementation */
/*************************************************/
void OpticalWaterLevelSensor::stateSet(state_t state){
	this->state = state;
}

const float OpticalWaterLevelSensor::mountpositionGet(void){
	return this->mountpositionMeters;
}

bool OpticalWaterLevelSensor::isValid(void){
	return this->state != state_t::unknown ? true : false;
}

bool OpticalWaterLevelSensor::isWet(void){
	return this->state == state_t::wet ? true : false;
}


/***********************************/
/*! WaterTank class implementation */
/***********************************/
double WaterTank::temperatureGet(void){

	this->temperature = 0; //ADC read and calculate temperature function

	if(temperature < 0.0){
		this->_isOK = false;
		this->stateSet(state_t::frozen);
	}
	else{
		this->_isOK = true;
		this->stateSet(state_t::liquid);
	}

	return this->temperature;
}

bool WaterTank::waterlevelSet(uint8_t & waterlevel){

	this->waterlevel = waterlevel;
	//todo: constrain to 0-100

	if (this->waterlevel < 10){
		this->_isOK = false;
	}
	else this->_isOK = true;

	return this->_isOK;
}

uint8_t WaterTank::waterlevelGet(void){
	return this->waterlevel;
}

bool WaterTank::checkStateOK(void){
	return this->_isOK;
}

WaterTank::state_t WaterTank::stateGet(void){
	return this->state;
}
void WaterTank::stateSet(state_t state){
	this->state = state;
}




///*! MoistureSensor template class implementation (Test only) */
//
//template <typename T> void MoistureSensor::rawreadingSet(T & _moisture_reading_raw){
//	this->moisture_reading_raw = _moisture_reading_raw;
//}
//
//
//
//
//
///*! AnalogMoistureSensor class implementation (Test only TBD) */
//
//uint8_t AnalogMoistureSensor::moistureCalculatePercent(void){
//	this->moisture_percent = this->moisture_reading_raw;
//	return this->moisture_percent;
//}



