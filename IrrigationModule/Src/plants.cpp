
#include <plants.h>



void Plant::moistureSetPercent(uint8_t _moisture){
	this->moisture = _moisture;
}

uint8_t Plant::moistureGetPercent(void){
	return this->moisture;
}

string Plant::nameGet(void){
	return this->name;
}
