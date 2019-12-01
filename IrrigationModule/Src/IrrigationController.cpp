


#include <IrrigationController.h>



bool Pump::isRunning(void){
	return this->_running;
}

void Pump::turnON(void){
	//TODO: add timing restrictions
	this->_running = true;
}

void Pump::turnOFF(void){
		this->_running = false;
}

uint32_t Pump::incrementRunningTime(void){
	return this->runningTimeSeconds++;
}

void Pump::resetRunningTime(void){
	this->runningTimeSeconds = 0;
}

uint32_t Pump::getTimeFromLastRun(void){
	return this->timeFromLastRunSeconds;
}




bool Tank::setTemperature(double temp){

	bool success = true;
	this->temperature = temp;

	if(temp < 0.0){
		this->_isOK = false;
	}
	else this->_isOK = true;

	return this->_isOK;
}

double Tank::getTemperature(void){
	return this->temperature;
}

bool Tank::setWaterLevel(uint8_t water_lvl){

	bool success = true;
	this->waterLevel = water_lvl;

	if (water_lvl < 10){
		this->_isOK = false;
	}
	else this->_isOK = true;

	return success;
}

uint8_t Tank::getWaterLevel(void){
	return this->waterLevel;
}

bool Tank::checkStateOK(void){
	return this->_isOK;
}
