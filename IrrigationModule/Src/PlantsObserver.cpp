
#include <PlantsObserver.h>



bool Plant::setMoisture(uint8_t p_moisture){
	bool success = true;

	this->moisture = p_moisture;

	return success;
}

uint8_t Plant::getMoisture(void){
	return this->moisture;
}

string Plant::getName(void){
	return this->name;
}

