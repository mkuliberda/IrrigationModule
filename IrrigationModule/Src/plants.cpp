
#include <plants.h>


/***********************************/
/*! Plants class implementation */
/***********************************/

void Plant::moisturePercentSet(const float & _soilmoisture){
	this->soilMoisture = _soilmoisture;
}

float Plant::moisturePercentGet(void){
	return this->soilMoisture;
}

string& Plant::nameGet(void){
	return this->name;
}

/***********************************/
/*! PlantsGroup class implementation */
/***********************************/

const uint8_t& PlantsGroup:: sectorGet(void){
	return this->sector;
}

bool PlantsGroup::plantAdd(const string & _name){
	bool success = true;
	return success;
}

bool update(const double & _dt){
	return true;
}

