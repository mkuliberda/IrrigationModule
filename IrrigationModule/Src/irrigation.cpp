


#include <irrigation.h>


void GenericPump::start(void){
	//TODO: add timing restrictions
	if(this->idletimeGetSeconds() > this->idletimeRequiredSeconds){
		this->stateSet(state_t::running);
		this->idletimeReset();
	}
	else{
		this->stateSet(state_t::waiting);
	}
}

void GenericPump::stop(void){

	if(this->stateGet() == state_t::running || this->stateGet() == state_t::init){
		this->stateSet(state_t::stopped);
		this->runtimeReset();
	}
}

GenericPump::state_t GenericPump::stateGet(void){
	return this->state;
}

void GenericPump::stateSet(state_t state){
	this->state = state;
}

bool GenericPump::isRunning(void){
	return stateGet() == state_t::running ? true : false;
}

void GenericPump::step(double dt){

	if(this->isRunning() == true){
		this->idletimeReset();
		this->runtimeIncrease(dt);
	}
	else{
		this->runtimeReset();
		this->idletimeIncrease(dt);
	}
}

void GenericPump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void GenericPump::runtimeIncrease(double dt){
	this->runtimeSeconds += dt;
}

double GenericPump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void GenericPump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void GenericPump::idletimeIncrease(double dt){
	this->idletimeSeconds += dt;
}

double GenericPump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}

string GenericPump::descriptionGet(void){
	return this->description;
}



double WaterTank::temperatureGet(void){

	//this->temperature = ADC read and calculate temperature function

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

bool WaterTank::waterlevelSet(uint8_t waterlevel){

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

const uint8_t WaterTank::waterlevelsAmountGet(void){
	return this->levels_amount;
}



void OpticalWaterLevelSensor::stateSet(state_t state){
	this->state = state;
}

const uint8_t OpticalWaterLevelSensor::mountpositionGet(void){
	return this->mount_position; //max <100
}

bool OpticalWaterLevelSensor::isValid(void){
	return this->state != state_t::unknown ? true : false;
}

bool OpticalWaterLevelSensor::isWet(void){
	return this->state == state_t::wet ? true : false;
}




template <typename T> void GenericMoistureSensor::rawreadingSet(T _moisture_reading_raw){
	this->moisture_reading_raw = _moisture_reading_raw;
}


uint8_t AnalogMoistureSensor::moistureCalculatePercent(void){
	this->moisture_percent = this->moisture_reading_raw;
	return this->moisture_percent;
}


//void Controller::run(void){
//
//}

