/*
 * freeRTOSTasks.cpp
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 */

/*Pelargonie co drugi dzieñ
 * Surfinie codziennie
 * Trawa raz na 3 dni
 */

#include <freeRTOSTasks.h>
#include <irrigation.h>
#include <utilities.h>
#include <plants.h>
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include <string>
#include "main.h"
#include "adc.h"
#include "nrf24l01.h"

#define PLANT1_ID 1
#define PLANT2_ID 2
#define PLANT3_ID 3
#define PLANT4_ID 4
#define SECTOR1_ID 1
#define SECTOR2_ID 2
#define SECTOR3_ID 3
#define WATERTANK1_ID 1

extern SemaphoreHandle_t xUserButtonSemaphore;
extern SemaphoreHandle_t xADCReadingsReadySemaphore;
extern xQueueHandle ADCValuesQueue;
extern xQueueHandle tank1StatusQueue;
extern xQueueHandle pumpsStatusQueue;
extern xQueueHandle sectorsStatusQueue;
extern xQueueHandle plantsHealthQueue;
extern xQueueHandle batteryStatusQueue;
extern xQueueHandle externalCommandsQueue;
extern xQueueHandle sysStatusQueue;
extern xQueueHandle serviceQueue;


void vADCReadTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
		ADC1_Start();
    	LEDToggle(9);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vLEDFlashTask( void *pvParameters )
{
	/*************************
	 * light led every second
	 *************************/
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	LEDToggle(10);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vUserButtonCheckTask(void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	if (UserButtonRead() == 1)
    	{
    		HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_SET);
    		xSemaphoreGive(xUserButtonSemaphore);
    	}
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}


void vIrrigationControlTask( void *pvParameters )
{

	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

	constexpr double tank1HeightMeters = 0.43;
	constexpr double tank1VolumeLiters = 5.0;
	constexpr float WLSensorHighPositionMeters = 0.12;
	constexpr float WLSensorLowPositionMeters = 0.38;

	constexpr struct gpio_s pump1gpio = {PUMP1_GPIO_Port, PUMP1_Pin};
	constexpr struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	constexpr struct gpio_s pump2gpio = {PUMP2_GPIO_Port, PUMP2_Pin};
	constexpr struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	constexpr struct gpio_s pump3gpio = {PUMP3_GPIO_Port, PUMP3_Pin};
	constexpr struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};
	constexpr struct gpio_s ds18b20_1gpio = {DS18B20_1_GPIO_Port, DS18B20_1_Pin};

	constexpr struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	constexpr struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};

	struct plant_s plant1 = {"Pelargonia1", PLANT1_ID, 0.0};
	struct plant_s plant2 = {"Surfinia1", PLANT2_ID, 0.0};
	struct plant_s plant3 = {"Surfinia2", PLANT3_ID, 0.0};
	struct plant_s plant4 = {"Trawa", PLANT4_ID, 0.0};

	struct servicecode_s errorcode;

	uint8_t rcvd_cmds_nbr = 0;

	double dt_seconds = xFrequencySeconds/1000.0f;

	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0; //8 bits per pump
	uint32_t sectorsStatus = 0;
	uint8_t sectorStatus[MAX_ENTITIES];
	bool sectorStatusReq[MAX_ENTITIES];
	uint16_t sector1ADCValue[1] = {1};
	uint16_t sector2ADCValue[2] = {2,3};
	uint16_t sector3ADCValue[1] = {4};
	uint16_t freeADCValue[4] = {5,6,7,8};
	uint16_t batteryADCValue[1] = {9};


	IrrigationSector *sector1 = new IrrigationSector(SECTOR1_ID);
	sector1->plantCreate(plant1.name, plant1.id);
	sector1->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector1->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector1->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector1->irrigationController->pBinPump->init(1, 4, 10, pump1gpio, pump1led);
	}


	IrrigationSector *sector2 = new IrrigationSector(SECTOR2_ID);
	sector2->plantCreate(plant2.name, plant2.id);
	sector2->plantCreate(plant3.name, plant3.id);
	sector2->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector2->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector2->irrigationController->pBinPump->init(2, 5, 15, pump2gpio, pump2led);
	}


	IrrigationSector *sector3 = new IrrigationSector(SECTOR3_ID);
	sector3->plantCreate(plant4.name, plant4.id);
	sector3->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector3->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector3->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector3->irrigationController->pBinPump->init(3, 5, 15, pump3gpio, pump3led);
	}


	WaterTank *tank1 = new WaterTank(tank1HeightMeters, tank1VolumeLiters, WATERTANK1_ID);
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(0).init(WLSensorHighPositionMeters, opticalwaterlevelsensor1gpio);
	}
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(1).init(WLSensorLowPositionMeters, opticalwaterlevelsensor2gpio);
	}
	if (tank1->temperatureSensorCreate(temperaturesensortype_t::ds18b20) == true){
		tank1->vTemperatureSensors.at(0).init(ds18b20_1gpio, &htim7);
	}


	bool sector1_irrigate = true; //TODO: false
	bool sector2_irrigate = true; //TODO: false
	bool sector3_irrigate = true; //TODO: false
	bool watertank1_valid = false;



    for( ;; )
    {
    	//TODO calculate dt_seconds based on real world period instead of a fixed one
    	rcvd_cmds_nbr = 0;
    	watertank1_valid = tank1->checkStateOK(dt_seconds, tank1Status);

    	if(xADCReadingsReadySemaphore != NULL)
    	{
		   if (xSemaphoreTake(xADCReadingsReadySemaphore, (portTickType)2) == pdTRUE)
		   {
				for (uint8_t i=0; i<9;i++){
					switch (i)
					{
					case 0:
						xQueueReceive(ADCValuesQueue, &sector1ADCValue[0], 0);
						break;
					case 1:
						xQueueReceive(ADCValuesQueue, &sector2ADCValue[0], 0);
						break;
					case 2:
						xQueueReceive(ADCValuesQueue, &sector2ADCValue[1], 0);
						break;
					case 3:
						xQueueReceive(ADCValuesQueue, &sector3ADCValue[0], 0);
						break;
					case 4:
						xQueueReceive(ADCValuesQueue, &freeADCValue[0], 0);
						break;
					case 5:
						xQueueReceive(ADCValuesQueue, &freeADCValue[1], 0);
						break;
					case 6:
						xQueueReceive(ADCValuesQueue, &freeADCValue[2], 0);
						break;
					case 7:
						xQueueReceive(ADCValuesQueue, &freeADCValue[3], 0);
						break;
					case 8:
						xQueueReceive(ADCValuesQueue, &batteryADCValue[0], 0);
						break;
					default:
						break;
					}
				}
		   }
    	}


		struct extcmd_s receivedcommands[EXTCMDS_BUFFER_LENGTH];
		struct extcmd_s rx;
    	while (xQueueReceive(externalCommandsQueue, &rx, ( TickType_t ) 10)){
    		receivedcommands[rcvd_cmds_nbr] = rx;
    		rcvd_cmds_nbr++;
    	}


    	for (uint8_t i=0; i < rcvd_cmds_nbr; i++)
    	{
			if(receivedcommands[i].cmd == externalcommand_t::requeststatus){

				switch(receivedcommands[i].target){
				case localtarget_t::watertank:
					xQueueOverwrite( tank1StatusQueue, &tank1Status);
					break;

				case localtarget_t::pump:
				   	pumpStateEncode(sector1->irrigationController->pBinPump->statusGet(), pumpsStatus);
				    pumpStateEncode(sector2->irrigationController->pBinPump->statusGet(), pumpsStatus);
				    pumpStateEncode(sector3->irrigationController->pBinPump->statusGet(), pumpsStatus);
				    xQueueOverwrite( pumpsStatusQueue, &pumpsStatus);
					break;

				case localtarget_t::plant:
					if		(receivedcommands[i].target_id == plant1.id){
						plant1.health = sector1->planthealthGet(plant1.id);
						xQueueSendToFront(plantsHealthQueue, &plant1.health, ( TickType_t ) 0);
					}
					else if	(receivedcommands[i].target_id == plant2.id){
						plant2.health = sector2->planthealthGet(plant2.id);
						xQueueSendToFront(plantsHealthQueue, &plant2.health, ( TickType_t ) 0);
					}
					else if	(receivedcommands[i].target_id == plant3.id){
						plant3.health = sector2->planthealthGet(plant3.id);
						xQueueSendToFront(plantsHealthQueue, &plant3.health, ( TickType_t ) 0);
					}
					else if	(receivedcommands[i].target_id == plant4.id){
						plant4.health = sector3->planthealthGet(plant4.id);
						xQueueSendToFront(plantsHealthQueue, &plant4.health, ( TickType_t ) 0);
					}
					else if (receivedcommands[i].target_id == 255){
						plant1.health = sector1->planthealthGet(plant1.id);
						xQueueSendToFront(plantsHealthQueue, &plant1.health, ( TickType_t ) 0);
						plant2.health = sector2->planthealthGet(plant2.id);
						xQueueSendToFront(plantsHealthQueue, &plant2.health, ( TickType_t ) 0);
						plant3.health = sector2->planthealthGet(plant3.id);
						xQueueSendToFront(plantsHealthQueue, &plant3.health, ( TickType_t ) 0);
						plant4.health = sector3->planthealthGet(plant4.id);
						xQueueSendToFront(plantsHealthQueue, &plant4.health, ( TickType_t ) 0);
					}
					break;

				case localtarget_t::powersupply:
					break;

				case localtarget_t::sys:
					break;

				case localtarget_t::sector:
					if		(receivedcommands[i].target_id == sector1->sectorGet()){
						sectorStatusReq[0] = true;
					}
					else if	(receivedcommands[i].target_id == sector2->sectorGet()){
						sectorStatusReq[1] = true;
					}
					else if	(receivedcommands[i].target_id == sector3->sectorGet()){
						sectorStatusReq[2] = true;
					}
					else if	(receivedcommands[i].target_id == 255){
						sectorStatusReq[0] = true;
						sectorStatusReq[1] = true;
						sectorStatusReq[2] = true;
					}
					break;

				default:
					break;
			}
			}else if(receivedcommands[i].cmd == externalcommand_t::enable){

				if (receivedcommands[i].target == localtarget_t::pump){

					switch (receivedcommands[i].target_id){
					case 1:
						if (watertank1_valid) sector1_irrigate = true;
						else sector1_irrigate = false;
						break;

					case 2:
						if (watertank1_valid) sector2_irrigate = true;
						else sector2_irrigate = false;
						break;

					case 3:
						if (watertank1_valid) sector3_irrigate = true;
						else sector3_irrigate = false;
						break;

					default:
						break;
					}
				}
			}else if(receivedcommands[i].cmd == externalcommand_t::disable){

				if (receivedcommands[i].target == localtarget_t::pump){

					switch (receivedcommands[i].target_id){
					case 1:
						sector1_irrigate = false;
						break;

					case 2:
						sector2_irrigate = false;
						break;

					case 3:
						sector3_irrigate = false;
						break;

					default:
						break;
					}
				}
			}

    	}

    	sectorStatus[0] = sector1->update(dt_seconds, sector1_irrigate, sector1ADCValue, 1);
    	sectorStatus[1] = sector2->update(dt_seconds, sector2_irrigate, sector2ADCValue, 2);
    	sectorStatus[2] = sector3->update(dt_seconds, sector3_irrigate, sector3ADCValue, 1);
    	sectorsStatus = sectorStatus[3]<<24 | sectorStatus[2]<<16 | sectorStatus[1]<<8 | sectorStatus[0];


    	if (sectorStatusReq[0] or sectorStatusReq[1] or sectorStatusReq[2] or sectorStatusReq[3]){
    		xQueueOverwrite(sectorsStatusQueue, &sectorsStatus);
    		sectorStatusReq[0] = false;
    		sectorStatusReq[1] = false;
    		sectorStatusReq[2] = false;
    		sectorStatusReq[3] = false;
    	}


    	if(xUserButtonSemaphore != NULL){
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE){
			   errorcode.reporter = localtarget_t::sector;
			   errorcode.id = 255;
			   errorcode.code = sectorsStatus;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = localtarget_t::watertank;
			   errorcode.id = tank1->idGet();
			   errorcode.code = tank1Status;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   pumpStateEncode(sector1->irrigationController->pBinPump->statusGet(), pumpsStatus);
			   pumpStateEncode(sector2->irrigationController->pBinPump->statusGet(), pumpsStatus);
			   pumpStateEncode(sector3->irrigationController->pBinPump->statusGet(), pumpsStatus);
			   errorcode.reporter = localtarget_t::pump;
			   errorcode.id = 255;
			   errorcode.code = pumpsStatus;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = localtarget_t::powersupply; //TODO
			   errorcode.id = 255;
			   errorcode.code = 0xffffffff;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = localtarget_t::sys; //TODO
			   errorcode.id = 255;
			   errorcode.code = 0xffffffff;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

		   }
    	}


    	LEDToggle(6);
		vTaskDelayUntil(&xLastWakeTime, xFrequencySeconds);
    }

    delete sector1; delete sector2; delete sector3;
    delete tank1;

}
void vStatusNotifyTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();
	servicecode_u rx;

    for( ;; )
    {
		while (xQueueReceive(serviceQueue, &rx.servicecode, ( TickType_t ) 10)){
			HAL_UART_Transmit(&huart4, rx.buffer, 6, 10);
			HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_RESET);
		}

        vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}
void vWirelessCommTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 0.05 * TASK_FREQ_MULTIPLIER; //<20Hz
	xLastWakeTime=xTaskGetTickCount();

	const struct gpio_s radio1ce = {NRF24_CE_GPIO_Port, NRF24_CE_Pin};
	const struct gpio_s radio1csn = {NRF24_NSS_GPIO_Port, NRF24_NSS_Pin};
	txframe_u radio1FrameTx;
	rxframe_u radio1FrameRx;
	struct extcmd_s cmd;
	bool radio1Configured = false;
	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0;
	struct plant_s plant;

	//TODO: remove for release
	uint8_t test_counter = 0;
	//-------------------------

	radio1FrameTx.values.start = commdirection_t::irm_to_rpi;
	radio1FrameRx.values.target_id = 0;
	radio1FrameRx.values.subcmd1 = 0;
	radio1FrameRx.values.subcmd2 = 0;
	radio1FrameRx.values.subcmd3 = 0;
	radio1FrameRx.values.subcmd4 = 0;
	//radio1FrameRx.values.free[23]; TODO
	radio1FrameRx.values.crc8 = 0;


	/* Receiver address */
	uint8_t TxAddress[] = {
		0xE7,
		0xE7,
		0xE7,
		0xE7,
		0xE7
	};
	/* My address */
	uint8_t MyAddress[] = {
		0x7E,
		0x7E,
		0x7E,
		0x7E,
		0x7E
	};

	/* NRF transmission status */
	NRF24L01_Transmit_Status_t transmissionStatus;

	NRF24L01 *radio1 = new NRF24L01();

	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	radio1->Init(&hspi2,radio1ce, radio1csn);
	radio1Configured = radio1->Config(32, 15, NRF24L01_OutputPower_M18dBm, NRF24L01_DataRate_2M);

	/* Set my address, 5 bytes */
	radio1->SetMyAddress(MyAddress);

	/* Set TX address, 5 bytes */
	radio1->SetTxAddress(TxAddress);

    for( ;; )
    {
    	if(radio1Configured == true){

			/* If data is ready on NRF24L01+ */
			while (radio1->DataReady()) {
				/* Get data from NRF24L01+ */
				radio1->GetPayload(radio1FrameRx.buffer);
				if (radio1FrameRx.values.start == commdirection_t::rpi_to_irm) //TODO: add crc
				{
					// Post received message.
					cmd.target = radio1FrameRx.values.target;
					cmd.target_id = radio1FrameRx.values.target_id;
					cmd.cmd = radio1FrameRx.values.cmd;
					cmd.subcmd1 = radio1FrameRx.values.subcmd1;
					cmd.subcmd1 = radio1FrameRx.values.subcmd2;
					cmd.subcmd1 = radio1FrameRx.values.subcmd3;
					cmd.subcmd1 = radio1FrameRx.values.subcmd4;
					xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				}
			}

			if (xQueueReceive( tank1StatusQueue, &tank1Status, 0 ) == pdPASS){
				radio1FrameTx.values.sender = localtarget_t::watertank;
				radio1FrameTx.values.sender_id = 1;
				radio1FrameTx.values.val.uint32 = tank1Status;
				radio1FrameTx.values.crc8 = 0;						//TODO
				radio1->TransmitPayload(radio1FrameTx.buffer);
				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

			}
			else if(xQueueReceive( pumpsStatusQueue, &pumpsStatus, 0 ) == pdPASS){
				radio1FrameTx.values.sender = localtarget_t::pump;
				radio1FrameTx.values.sender_id = 255;
				radio1FrameTx.values.val.uint32 = pumpsStatus;
				radio1FrameTx.values.crc8 = 0;						//TODO
				radio1->TransmitPayload(radio1FrameTx.buffer);
				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
				test_counter++;
			}

			while(xQueueReceive( plantsHealthQueue, &plant, 0 )){
				radio1FrameTx.values.sender = localtarget_t::plant;
				radio1FrameTx.values.sender_id = plant.id;
				radio1FrameTx.values.val.float32 = plant.health;
				//radio1FrameTx.values.desc							//TODO
				radio1FrameTx.values.crc8 = 0;						//TODO
				radio1->TransmitPayload(radio1FrameTx.buffer);
				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

			}

			/* Go back to RX mode */
			radio1->PowerUpRx();

			/*switch(test_counter){

			case 0:
				cmd.target = localtarget_t::pump;
				cmd.target_id = 2;
				cmd.cmd = externalcommand_t::enable;
				xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				break;
			case 1:
				cmd.target = localtarget_t::watertank;
				cmd.target_id = 1;
				cmd.cmd = externalcommand_t::requeststatus;
				xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				break;
			case 2:
				cmd.target = localtarget_t::plant;
				cmd.target_id = 255;
				cmd.cmd = externalcommand_t::requeststatus;
				xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				break;
			case 3:
				cmd.target = localtarget_t::plant;
				cmd.target_id = 1;
				cmd.cmd = externalcommand_t::requeststatus;
				xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				break;
			case 4:
				cmd.target = localtarget_t::pump;
				cmd.target_id = 2;
				cmd.cmd = externalcommand_t::disable;
				xQueueSendToFront(externalCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
				break;
			}

			if (test_counter < 10) test_counter++;
			else test_counter = 0;*/
    	}

    	LEDToggle(7);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

    delete radio1;

}



