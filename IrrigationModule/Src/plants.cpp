
#include <plants.h>


/***********************************/
/*! Plants class implementation */
/***********************************/

void Plant::moisturePercentSet(const float & _soilmoisture){
	this->soilMoisture = _soilmoisture;
}

float& Plant::moisturePercentGet(void){
	return this->soilMoisture;
}

string& Plant::nameGet(void){
	return this->name;
}

void Plant::nameChange(const string & _new_name){
	this->name.assign(_new_name);
}

const uint32_t& Plant::idGet(void){
	return this->id;
}


/***********************************/
/*! IrrigationSector class implementation */
/***********************************/

const uint8_t& IrrigationSector:: sectorGet(void){
	return this->sector;
}

bool IrrigationSector::plantCreate(const string & _name, const uint32_t & _id){

	bool success = true;

	if (this->plantsCount <= this->plantsCountMax)
	{
		Plant temp_plant(_name, _id);
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
	//TODO:implement this for moisture sensors other than ADC DMA
	return 0;
}

uint8_t IrrigationSector::update(const double & _dt, const bool & _activate_watering, uint16_t *_raw_adc_values_array, const uint8_t & _raw_adc_values_cnt){

	uint8_t status = 0;

	for (uint8_t i=0; i<_raw_adc_values_cnt; i++){
		this->irrigationController->vDMAMoistureSensor.at(i).rawUpdate(_raw_adc_values_array[i]);
		this->vPlants.at(i).moisturePercentSet(this->irrigationController->vDMAMoistureSensor.at(i).percentGet());
	}

	//get status of pump
	status = this->irrigationController->update(_dt, _activate_watering);

	return status;
}

uint8_t& IrrigationSector::plantscountGet(void){
	return this->plantsCount;
}

float IrrigationSector::planthealthGet(const string & _name){

	float tempHealth = -1000.0;

	if(this->plantsCount > 0)
	{
		for (uint8_t i = 0; i < this->plantsCount; i++)
		{
			if (_name.compare(this->vPlants.at(i).nameGet())){
				tempHealth = this->vPlants.at(i).moisturePercentGet();
				return tempHealth;
			}
		}
	}

	return tempHealth;
}

float IrrigationSector::planthealthGet(const uint32_t & _id){

	float tempHealth = -1000.0;

	if(this->plantsCount > 0)
	{
		for (uint8_t i = 0; i < this->plantsCount; i++)
		{
			if (this->vPlants.at(i).idGet() == _id){
				tempHealth = this->vPlants.at(i).moisturePercentGet();
				return tempHealth;
			}
		}
	}

	return tempHealth;
}


