/*
 * PlantsObserver.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PLANTS_H_
#define PLANTS_H_

#include <string>

using namespace std;

class Plant{

public:

	Plant(string name):
	name(name),
	moisture(0)
	{};

	~Plant();

	void moistureSetPercent(uint8_t _moisture);
	uint8_t moistureGetPercent(void);
	string nameGet(void);

private:
	string name;
	uint8_t moisture;
};


#endif /* PLANTS_H_ */
