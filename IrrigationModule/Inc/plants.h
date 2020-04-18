/*
 * Plants.h
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

#define NAME_LENGTH 20

struct plant_s{
	char name[NAME_LENGTH];
	uint8_t id;
	float health;
};

struct sectorstatus_s {
	uint8_t id;
	uint32_t state;
	std::string plants;
};


class Plant{

private:

	std::string name;
	float soilMoisture;
	const uint8_t id;

public:

	Plant(const std::string & _name, const uint8_t & _id):
	name(_name),
	soilMoisture(-1000),
	id(_id)
	{};

	~Plant(){};

	void 					moisturePercentSet(const float & _soilmoisture);
	float& 					moisturePercentGet(void);
	std::string& 			nameGet(void);
	void					nameChange(const std::string & _new_name);
	const uint8_t&			idGet(void);

};

class IrrigationSector{

private:

	const uint8_t 	id;
	const uint8_t 	plantsCountMax = 20;
	uint8_t 		plantsCount;
	std::vector<Plant> 	vPlants;
	uint8_t 		status;
	bool			water_plants;

public:

	PumpController *irrigationController;

	IrrigationSector(const uint8_t & _id):
	id(_id),
	plantsCount(0),
	status(0),
	water_plants(false)
	{
		irrigationController = new PumpController();
	};

	~IrrigationSector(){
		delete irrigationController;
	}

	const uint8_t& 			sectorGet(void);
	bool 					plantCreate(const std::string & _name, const uint8_t & _id);
	uint8_t& 				update(const double & _dt);
	uint8_t& 				update(const double & _dt, const bool & _activate_watering);
	uint8_t& 				update(const double & _dt, const bool & _activate_watering, uint16_t *_raw_adc_values_array, const uint8_t & _raw_adc_values_cnt);
	uint8_t& 				plantscountGet(void);
	float 					planthealthGet(const std::string & _name);
	float 					planthealthGet(const uint8_t & _id);
	void					measurementsSet(uint16_t *_raw_adc_values_array, const uint8_t & _raw_adc_values_cnt);
	uint8_t& 				statusGet(void);
	pumpstate_t 			pumpstateGet(void);
//	struct pumpstatus_s&	pumpstatusGet(void); TODO
	void					wateringSet(const bool & _activate_watering);
	bool&					wateringGet(void);
};

#endif /* PLANTS_H_ */
