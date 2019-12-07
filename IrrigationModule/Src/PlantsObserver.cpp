
#include <PlantsObserver.h>



void Plant::setMoisture(uint8_t _moisture){
	this->moisture = _moisture;
}

uint8_t Plant::getMoisture(void){
	return this->moisture;
}

string Plant::getName(void){
	return this->name;
}

