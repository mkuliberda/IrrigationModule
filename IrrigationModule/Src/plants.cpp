
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

uint8_t IrrigationSector::update(const double & _dt, const bool & _activate_watering){
	//TODO:implement this
	return 0;
}

uint8_t IrrigationSector::update(const double & _dt, const bool & _activate_watering, uint16_t *_raw_adc_values_array, const uint8_t & _raw_adc_values_cnt){

	//TODO: implement this
	uint8_t status = 0;
	//		this->irrigationController->vDMAMoistureSensor[0].rawUpdate(adcValues[0]);
	//set plants, moisture
	//get status of pump
	status = this->irrigationController->update(_dt, _activate_watering);

	return status;
}

uint8_t& IrrigationSector::plantscountGet(void){
	return this->plantsCount;
}

struct plant_s IrrigationSector::planthealthGet(const string & _name){

	struct plant_s temp_plant = {"no match" , -1};

	if(this->plantsCount > 0)
	{
		for (uint8_t i = 0; i < (this->plantsCount+1); i++)
		{
			if (this->vPlants[i].nameGet() == _name){
				temp_plant.name = this->vPlants[i].nameGet();
				temp_plant.soilmoisturePercent = this->vPlants[i].moisturePercentGet();
			}
		}
	}
	//TODO: implement this
	return temp_plant;
}


