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

	~Plant();

	void moisturePercentSet(const float & _soilmoisture);
	float moisturePercentGet(void);
	string& nameGet(void);

};

class PlantsGroup{

private:

	const uint8_t sector;
	const uint8_t plantsAmountMax = 20;
	uint8_t plantsAmountCount;
	vector<Plant> vPlants;

public:

	PumpController *irrigationController;

	PlantsGroup(const uint8_t & _sector):
	sector(_sector),
	plantsAmountCount(0)
	{
		irrigationController = new PumpController();
	};

	~PlantsGroup(){
		delete irrigationController;
	}

	const uint8_t& sectorGet(void);
	bool plantAdd(const string & _name);
	bool update(const double & _dt);

};

#endif /* PLANTS_H_ */
