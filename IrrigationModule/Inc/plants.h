/*
 * PlantsObserver.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PLANTS_H_
#define PLANTS_H_

#include "stm32f3xx_hal.h"
#include <string>
#include "main.h"
#include "gpio.h"
#include "tim.h"
#include <array>
#include <vector>
#include <memory>
#include <utilities.h>
#include <bitset>
#include <numeric>
#include <irrigation.h>

struct plant_s{
	string name;
	float soilmoisturePercent;
};

using namespace std;

class Plant{

private:

	string name;
	float soilMoisture;

public:

	Plant(const string & _name):
	name(_name),
	soilMoisture(0)
	{};

	~Plant(){};

	void moisturePercentSet(const float & _soilmoisture);
	float moisturePercentGet(void);
	string& nameGet(void);

};

class IrrigationSector{

private:

	const uint8_t sector;
	const uint8_t plantsCountMax = 20;
	uint8_t plantsCount;
	vector<Plant> vPlants;

public:

	PumpController *irrigationController;

	IrrigationSector(const uint8_t & _sector):
	sector(_sector),
	plantsCount(0)
	{
		irrigationController = new PumpController();
	};

	~IrrigationSector(){
		delete irrigationController;
	}

	const uint8_t& sectorGet(void);
	bool plantCreate(const string & _name);
	uint8_t update(const double & _dt, const bool & _activate_watering);
	uint8_t update(const double & _dt, const bool & _activate_watering, uint16_t *_raw_adc_values_array, const uint8_t & _raw_adc_values_cnt);
	uint8_t& plantscountGet(void);
	struct plant_s planthealthGet(const string & _name);


};

#endif /* PLANTS_H_ */
