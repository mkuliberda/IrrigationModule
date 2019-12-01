/*
 * PlantsObserver.h
 *
 *  Created on: 28.11.2019
 *      Author: Mati
 */

#ifndef PLANTSOBSERVER_H_
#define PLANTSOBSERVER_H_

#include <string>

using namespace std;

class Plant{
	Plant(string p_name){this->name = p_name;};
	~Plant();

public:
	bool setMoisture(uint8_t p_moisture);
	uint8_t getMoisture(void);
	string getName(void);

private:
	string name;
	uint8_t moisture;
};


#endif /* PLANTSOBSERVER_H_ */
