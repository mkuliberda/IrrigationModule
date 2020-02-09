/*
 * wireless_base.h
 *
 *  Created on: 25.01.2020
 *      Author: Mati
 */

#ifndef COMMUNICATION_BASE_H_
#define COMMUNICATION_BASE_H_

#ifndef RPI
#include "stm32f3xx_hal.h"
#else

#endif

#define RPI_IRM 		0xAA
#define IRM_RPI 		0xBB

enum class communicationtype_t{
	undetermined,
	point_to_point,
	wifi
};

enum class commdirection_t{
	undefined,
	rpi_to_irm = RPI_IRM,
	irm_to_rpi = IRM_RPI,
	max = 255
};

enum class externalcommand_t{
	none,
	disable,
	enable,
	requeststatus,
	antifreeze,
	max = 255
};

enum class localtarget_t{
	none,
	plant,
	watertank,
	pump,
	sector,
	sys,
	powersupply,
	max = 255
};

enum class remotetarget_t{
	none,
	max = 255
};

struct servicecode_s{
	localtarget_t reporter;
	uint8_t id;
	uint32_t code;
};

struct wframeRx_s {
	commdirection_t start;
	localtarget_t target;
	uint8_t target_id;
	externalcommand_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
	uint8_t subcmd3;
	uint8_t subcmd4;
	uint8_t free[23];
	uint8_t crc8;
};

union val_u{
	float float32;
	uint32_t uint32;
};

union USART_Buffer32{
	uint32_t status;
	uint8_t buffer[4];
};

struct wframeTx_s {
	commdirection_t start;
	localtarget_t sender;
	uint8_t sender_id;
	union val_u val; 				//<<Make sure this is 32bit
	char  desc[24];
	uint8_t crc8;
};

struct extcmd_s{
	localtarget_t target;
	uint8_t target_id;
	externalcommand_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
	uint8_t subcmd3;
	uint8_t subcmd4;
};

union txframe_u{
	uint8_t 	buffer[32];
	wframeTx_s	values;
};

union rxframe_u{
	uint8_t 	buffer[32];
	wframeRx_s 	values;
};

union servicecode_u{
	servicecode_s servicecode;
	uint8_t buffer[6];
};


class Wireless{

protected:

	communicationtype_t 			commType;

	communicationtype_t& 			typeGet(void);


public:

	Wireless(){};

	virtual ~Wireless(){};

};




#endif /* COMMUNICATION_BASE_H_ */
