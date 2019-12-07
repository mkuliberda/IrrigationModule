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

public:

	Plant(string name):
	name(name),
	moisture(0)
	{};

	~Plant();

	void setMoisture(uint8_t _moisture);
	uint8_t getMoisture(void);
	string getName(void);

private:
	string name;
	uint8_t moisture;
};


#endif /* PLANTSOBSERVER_H_ */
