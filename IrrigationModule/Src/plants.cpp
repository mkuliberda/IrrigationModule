
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
/*! IrrigationSector class implementation */
/***********************************/

const uint8_t& IrrigationSector:: sectorGet(void){
	return this->sector;
}

bool IrrigationSector::plantCreate(const string & _name){

	bool success = true;

	if (this->plantsCount < (this->plantsCountMax + 1))
	{
		Plant temp_plant(_name);
		this->vPlants.push_back(temp_plant);
		this->plantsCount++;
	}
	else
	{
		success = false;
	}

	return success;
}

bool IrrigationSector::update(const double & _dt){
	//TODO:implement this
	return true;
}


