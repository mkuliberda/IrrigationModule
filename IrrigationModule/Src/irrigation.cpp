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
	return stateGet() == pumpState_t::running ? true : false;
}

void Pump::run(const double & _dt){

}


/************************************/
/*! BinaryPump class implementation */
/************************************/

bool BinaryPump::init(const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led){
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = _idletimeRequiredSeconds;
	this->runtimeLimitSeconds = _runtimeLimitSeconds;
	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpState_t::stopped);

	return this->isRunning();
}

void BinaryPump::run(const double & _dt, const bool & _cmd_start, bool & cmd_consumed){

	switch (this->stateGet())
	{
	case pumpState_t::init:
		this->stop();
		cmd_consumed = true;
		break;

	case pumpState_t::waiting:
		this->idletimeIncrease(_dt);
		if(this->idletimeGetSeconds() > this->idletimeRequiredSeconds){
			if (this->start() == true ) cmd_consumed = true;
		}

		break;

	case pumpState_t::stopped:
		this->idletimeIncrease(_dt);
		if((_cmd_start == true) && (cmd_consumed == false) && (this->idletimeGetSeconds() > this->idletimeRequiredSeconds)){
			if (this->start() == true ) cmd_consumed = true;
		}
		else if((_cmd_start == true) && (cmd_consumed == false) && (this->idletimeGetSeconds() <= this->idletimeRequiredSeconds)){
			this->stateSet(pumpState_t::waiting);
		}
		break;

	case pumpState_t::running:
		this->runtimeIncrease(_dt);
		if(_cmd_start == true){
			cmd_consumed = true;
		}
		else{
			if(this->stop() == true) cmd_consumed = true;
		}
		if(this->runtimeGetSeconds() > this->runtimeLimitSeconds && this->forced == false) this->stop();
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
		this->stateSet(pumpState_t::running);
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
		this->stateSet(pumpState_t::stopped);
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
	this->stateSet(pumpState_t::running);
	this->forced = true;
}
void BinaryPump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpState_t::stopped);
	this->forced = true;
}


pumpState_t BinaryPump::stateGet(void){
	return this->state;
}

void BinaryPump::stateSet(const pumpState_t & _state){
	this->state = _state;
}

void BinaryPump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void BinaryPump::runtimeIncrease(const double & _dt){
	this->runtimeSeconds += _dt;
}

double BinaryPump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void BinaryPump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void BinaryPump::idletimeIncrease(const double & _dt){
	this->idletimeSeconds += _dt;
}

double BinaryPump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}




/******************************************/
/*! WaterLevelSensor class implementation */
/******************************************/
bool WaterLevelSensor::init(const waterlevelsensorsubtype_t & _subtype){
	this->subtype = subtype;
	return true;
}

waterlevelsensorsubtype_t  WaterLevelSensor::subtypeGet(void){
	return this->subtype;
}


/*************************************************/
/*! OpticalWaterLevelSensor class implementation */
/*************************************************/
bool OpticalWaterLevelSensor::init(const waterlevelsensorsubtype_t & _subtype, const struct gpio_s & _pinout){
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->subtype = _subtype;
	this->read();
	return true;
}

const float OpticalWaterLevelSensor::mountpositionGet(void){
	return this->mountpositionMeters;
}

void OpticalWaterLevelSensor::read(void){
	if (HAL_GPIO_ReadPin(this->pinout.port, this->pinout.pin) == GPIO_PinState::GPIO_PIN_SET) this->state = fixedwaterlevelsensorState_t::dry;
	else this->state = fixedwaterlevelsensorState_t::wet;
}

bool OpticalWaterLevelSensor::isValid(void){
	return this->state != fixedwaterlevelsensorState_t::undetermined ? true : false;
}

bool OpticalWaterLevelSensor::isWet(void){
	this->read();
	return this->state == fixedwaterlevelsensorState_t::wet ? true : false;
}


/***********************************/
/*! WaterTank class implementation */
/***********************************/

bool WaterTank::init(const array<float, 10> & _fixedwlsposition){

	using index_t = array<int, 10>::size_type;
	for (index_t i{ 0 }; i < _fixedwlsposition.size(); ++i)
	{
		if ( _fixedwlsposition[i] > 0.0f)
		{
			OpticalWaterLevelSensor temp_WLSensor(_fixedwlsposition[i]);
			this->vWLSensors.push_back(temp_WLSensor);
		}
	}

	return true;
}

double WaterTank::temperatureGet(void){

	this->temperature = 12.7; //DS18B20 read function TBD

	if(temperature < 0.0){
		this->stateSet(contentstate_t::frozen);
	}
	else{
		this->stateSet(contentstate_t::liquid);
	}

	return this->temperature;
}

bool WaterTank::waterlevelSet(const contentlevel_t & _waterlevel){

	bool success = false;
	this->waterlevel = _waterlevel;
	//todo: constrain to 0-100
	return success;
}

WaterTank::contentlevel_t WaterTank::waterlevelGet(void){
	return this->waterlevel;
}

void WaterTank::stateSet(const contentstate_t & _state){
	this->waterstate = _state;
}

WaterTank::contentstate_t WaterTank::stateGet(void){
	return this->waterstate;
}

bool WaterTank::checkStateOK(uint32_t & errcodeBitmask){

	uint8_t waterlevel = 0;

	//TODO:loop over all avbl sensors
	if(this->vWLSensors[0].isWet()){
		if(this->waterlevelConvertToPercent(this->vWLSensors[0].mountpositionGet()) > waterlevel){
			waterlevel = this->vWLSensors[0].mountpositionGet();
		}
	}

	return this->_isOK;
}

float WaterTank::waterlevelConvertToPercent(const float & _valMeters){
	return _valMeters/this->tankheightMeters*100;
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



