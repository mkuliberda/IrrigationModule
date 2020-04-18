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
#include "msg_definitions_irrigation.h"
#include "power.h"


#define PUMP1_ID 0
#define PUMP2_ID 1
#define PUMP3_ID 2
#define PLANT1_ID 0
#define PLANT2_ID 1
#define PLANT3_ID 2
#define PLANT4_ID 3
#define PLANT5_ID 4
#define PLANT6_ID 5
#define PLANT7_ID 6
#define PLANT8_ID 7
#define SECTOR1_ID 0
#define SECTOR2_ID 1
#define SECTOR3_ID 2
#define WATERTANK1_ID 0
#define AVBL_SECTORS 3
#define BATTERY1_ID 0

extern SemaphoreHandle_t xUserButtonSemaphore;
extern SemaphoreHandle_t xADCReadingsReadySemaphore;
extern xQueueHandle adcValuesQueue;
extern xQueueHandle tanksStatusQueue;
extern xQueueHandle pumpsStatusQueue;
extern xQueueHandle sectorsStatusQueue;
extern xQueueHandle plantsHealthQueue;
extern xQueueHandle batteryStatusQueue;
extern xQueueHandle extCommandsQueue;
extern xQueueHandle confirmationsQueue;
extern xQueueHandle sysStatusQueue;
extern xQueueHandle serviceQueue;
extern xQueueHandle singleValsQueue;
extern xQueueHandle batteryQueue;

using namespace std;

bool handleConfirmation(IrrigationSector &_sector);

void vADCReadTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
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
	constexpr portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vUserButtonCheckTask(void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 0.5 * TASK_FREQ_MULTIPLIER; //<2Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	if (UserButtonRead() == 1)
    	{
    		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
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

	constexpr double tank1_height_meters = 0.43;
	constexpr double tank1_volume_liters = 5.0;
	constexpr float wls_high_pos_meters = 0.12;
	constexpr float wls_low_pos_meters = 0.38;
	constexpr uint32_t pump_maxruntime_seconds = 60;
	constexpr uint32_t pump_breaktime_seconds = 20;


	constexpr struct gpio_s pump1gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP1_IN1_Pin};
	constexpr struct gpio_s pump1gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP1_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump1gpio = {pump1gpio_in1, pump1gpio_in2};
	constexpr struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	constexpr struct gpio_s pump1fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_ULT_Pin};
	constexpr struct gpio_s pump1mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_EEP_Pin};

	constexpr struct gpio_s pump2gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP2_IN1_Pin};
	constexpr struct gpio_s pump2gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP2_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump2gpio = {pump2gpio_in1, pump2gpio_in2};
	constexpr struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	constexpr struct gpio_s pump2fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_ULT_Pin};
	constexpr struct gpio_s pump2mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_EEP_Pin};

	constexpr struct gpio_s pump3gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP3_IN1_Pin};
	constexpr struct gpio_s pump3gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP3_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump3gpio = {pump3gpio_in1, pump3gpio_in2};
	constexpr struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};
	constexpr struct gpio_s pump3fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_2_ULT_Pin};
	constexpr struct gpio_s pump3mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_2_EEP_Pin};

	constexpr struct gpio_s ds18b20_1gpio = {DS18B20_1_GPIO_Port, DS18B20_1_Pin};

	constexpr struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	constexpr struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};


	struct plant_s plant1 = {"Sensor1", PLANT1_ID, 0.0};
	struct plant_s plant2 = {"Sensor2", PLANT2_ID, 0.0};
	struct plant_s plant3 = {"Sensor3", PLANT3_ID, 0.0};
	struct plant_s plant4 = {"Sensor4", PLANT4_ID, 0.0};
	/*struct plant_s plant5 = {"Sensor5", PLANT5_ID, 0.0};
	struct plant_s plant6 = {"Sensor6", PLANT6_ID, 0.0};
	struct plant_s plant7 = {"Sensor7", PLANT7_ID, 0.0};
	struct plant_s plant8 = {"Sensor8", PLANT8_ID, 0.0};*/

	struct servicecode_s errorcode;
	//struct singlevalue_s single_val;
	struct battery_s battery;

	struct cmd_s received_commands[EXTCMDS_BUFFER_LENGTH];
	uint8_t rcvd_cmds_nbr = 0;

	double dt_seconds = xFrequencySeconds/1000.0f;

	struct tankstatus_s tank1_status = {WATERTANK1_ID, 0};
	uint32_t pumps_status = 0; //8 bits per pump
	uint32_t encoded_sectors_status = 0;
	uint8_t sector_status[MAX_ENTITIES];
	bool sector_status_requested[MAX_ENTITIES];
	bool cmd_confirmation_required[MAX_ENTITIES];
	uint16_t sector1_adc_value[1] = {1};
	uint16_t sector2_adc_value[2] = {2,3};
	uint16_t sector3_adc_value[1] = {4};
	uint16_t free_adc_value[4] = {5,6,7,8};
	uint16_t battery_adc_value[1] = {9};
	bool watertank1_valid = false;
	IrrigationSector sector[AVBL_SECTORS] = {{SECTOR1_ID}, {SECTOR2_ID}, {SECTOR3_ID}};


	sector[0].plantCreate(plant1.name, plant1.id);
	sector[0].irrigationController->modeSet(pumpcontrollermode_t::external);
	sector[0].irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[0].irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[0].irrigationController->p8833Pump->init(PUMP1_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump1gpio, pump1led, pump1fault, pump1mode);
	}


	sector[1].plantCreate(plant2.name, plant2.id);
	sector[1].plantCreate(plant3.name, plant3.id);
	sector[1].irrigationController->modeSet(pumpcontrollermode_t::external);
	sector[1].irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector[1].irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[1].irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[1].irrigationController->p8833Pump->init(PUMP2_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump2gpio, pump2led, pump2fault, pump2mode);
	}


	sector[2].plantCreate(plant4.name, plant4.id);
	sector[2].irrigationController->modeSet(pumpcontrollermode_t::external);
	sector[2].irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[2].irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[2].irrigationController->p8833Pump->init(PUMP3_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump3gpio, pump3led, pump3fault, pump3mode);
	}


	WaterTank *tank1 = new WaterTank(tank1_height_meters, tank1_volume_liters, WATERTANK1_ID);
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(0).init(wls_high_pos_meters, opticalwaterlevelsensor1gpio);
	}
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(1).init(wls_low_pos_meters, opticalwaterlevelsensor2gpio);
	}
	if (tank1->temperatureSensorCreate(temperaturesensortype_t::ds18b20) == true){
		tank1->vTemperatureSensors.at(0).init(ds18b20_1gpio, &htim7);
	}


    for( ;; )
    {
    	//TODO calculate dt_seconds based on real world period instead of a fixed one
    	rcvd_cmds_nbr = 0;
    	watertank1_valid = tank1->checkStateOK(dt_seconds, tank1_status.state);

    	if(xADCReadingsReadySemaphore != NULL)
    	{
		   if (xSemaphoreTake(xADCReadingsReadySemaphore, (portTickType)2) == pdTRUE)
		   {
				for (uint8_t i=0; i<9;++i){
					switch (i)
					{
					case 0:
						xQueueReceive(adcValuesQueue, &sector1_adc_value[0], 0);
						break;
					case 1:
						xQueueReceive(adcValuesQueue, &sector2_adc_value[0], 0);
						break;
					case 2:
						xQueueReceive(adcValuesQueue, &sector2_adc_value[1], 0);
						break;
					case 3:
						xQueueReceive(adcValuesQueue, &sector3_adc_value[0], 0);
						break;
					case 4:
						xQueueReceive(adcValuesQueue, &free_adc_value[0], 0);
						break;
					case 5:
						xQueueReceive(adcValuesQueue, &free_adc_value[1], 0);
						break;
					case 6:
						xQueueReceive(adcValuesQueue, &free_adc_value[2], 0);
						break;
					case 7:
						xQueueReceive(adcValuesQueue, &free_adc_value[3], 0);
						break;
					case 8:
						xQueueReceive(adcValuesQueue, &battery_adc_value[0], 0);
						break;
					default:
						break;
					}
				}
				sector[0].measurementsSet(sector1_adc_value, 1);
				sector[1].measurementsSet(sector2_adc_value, 2);
				sector[2].measurementsSet(sector3_adc_value, 1);
		   }
    	}


    	while (xQueueReceive(extCommandsQueue, &received_commands[rcvd_cmds_nbr], ( TickType_t ) 10)){
    		rcvd_cmds_nbr++;
    	}


    	for (uint8_t i=0; i < rcvd_cmds_nbr; i++)
    	{
 			if(received_commands[i].cmd == command_t::GetStatus){

				switch(received_commands[i].target){
				case target_t::Tank:
					xQueueOverwrite( tanksStatusQueue, &tank1_status);
					break;

				case target_t::Pump:
				   	pumpStateEncode(sector[0].irrigationController->p8833Pump->statusGet(), pumps_status);
				    pumpStateEncode(sector[1].irrigationController->p8833Pump->statusGet(), pumps_status);
				    pumpStateEncode(sector[2].irrigationController->p8833Pump->statusGet(), pumps_status);
				    xQueueOverwrite( pumpsStatusQueue, &pumps_status);
					break;

				case target_t::Plant:
					if		(received_commands[i].target_id == plant1.id){
						plant1.health = sector[0].planthealthGet(plant1.id);
						xQueueSendToFront(plantsHealthQueue, &plant1.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant2.id){
						plant2.health = sector[1].planthealthGet(plant2.id);
						xQueueSendToFront(plantsHealthQueue, &plant2.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant3.id){
						plant3.health = sector[1].planthealthGet(plant3.id);
						xQueueSendToFront(plantsHealthQueue, &plant3.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant4.id){
						plant4.health = sector[2].planthealthGet(plant4.id);
						xQueueSendToFront(plantsHealthQueue, &plant4.health, ( TickType_t ) 0);
					}
					else if (received_commands[i].target_id == 255){
						plant1.health = sector[0].planthealthGet(plant1.id);
						xQueueSendToFront(plantsHealthQueue, &plant1.health, ( TickType_t ) 0);
						plant2.health = sector[1].planthealthGet(plant2.id);
						xQueueSendToFront(plantsHealthQueue, &plant2.health, ( TickType_t ) 0);
						plant3.health = sector[1].planthealthGet(plant3.id);
						xQueueSendToFront(plantsHealthQueue, &plant3.health, ( TickType_t ) 0);
						plant4.health = sector[2].planthealthGet(plant4.id);
						xQueueSendToFront(plantsHealthQueue, &plant4.health, ( TickType_t ) 0);
					}
					break;

				case target_t::Power:
					break;

				case target_t::System:
					break;

				case target_t::Sector:
					if		(received_commands[i].target_id == sector[0].sectorGet()){
						sector_status_requested[0] = true;
					}
					else if	(received_commands[i].target_id == sector[1].sectorGet()){
						sector_status_requested[1] = true;
					}
					else if	(received_commands[i].target_id == sector[2].sectorGet()){
						sector_status_requested[2] = true;
					}
					else if	(received_commands[i].target_id == 255){
						sector_status_requested[0] = true;
						sector_status_requested[1] = true;
						sector_status_requested[2] = true;
					}
					break;

				case target_t::All:
					xQueueOverwrite( tanksStatusQueue, &tank1_status); //TODO: implement this, for test now
					plant1.health = sector[0].planthealthGet(plant1.id);
					xQueueSendToFront(plantsHealthQueue, &plant1, ( TickType_t ) 0);
					plant2.health = sector[1].planthealthGet(plant2.id);
					xQueueSendToFront(plantsHealthQueue, &plant2, ( TickType_t ) 0);
					plant3.health = sector[1].planthealthGet(plant3.id);
					xQueueSendToFront(plantsHealthQueue, &plant3, ( TickType_t ) 0);
					plant4.health = sector[2].planthealthGet(plant4.id);
					xQueueSendToFront(plantsHealthQueue, &plant4, ( TickType_t ) 0);
					sector_status_requested[0] = true;
					sector_status_requested[1] = true;
					sector_status_requested[2] = true;
					HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
					break;

				default:
					break;
			}
			}
			else if(received_commands[i].cmd == command_t::Start){

				if (received_commands[i].target == target_t::Sector){

					switch (received_commands[i].target_id){
					case SECTOR1_ID:
						if (watertank1_valid){
							sector[0].wateringSet(true);
							cmd_confirmation_required[0] = true;
						}
						else sector[0].wateringSet(false);
						break;

					case SECTOR2_ID:
						if (watertank1_valid){
							sector[1].wateringSet(true);
							cmd_confirmation_required[1] = true;
						}
						else sector[1].wateringSet(false);
						break;

					case SECTOR3_ID:
						if (watertank1_valid){
							sector[2].wateringSet(true);
							cmd_confirmation_required[2] = true;
						}
						else sector[2].wateringSet(false);
						break;

					default:
						break;
					}
				}
			}
			else if(received_commands[i].cmd == command_t::Stop){

				if (received_commands[i].target == target_t::Sector){

					switch (received_commands[i].target_id){
					case SECTOR1_ID:
						sector[0].wateringSet(false);
						cmd_confirmation_required[0] = true;
						break;

					case SECTOR2_ID:
						sector[1].wateringSet(false);
						cmd_confirmation_required[1] = true;
						break;

					case SECTOR3_ID:
						sector[2].wateringSet(false);
						cmd_confirmation_required[2] = true;
						break;

					default:
						break;
					}
				}
			}

    	}


    	for (uint8_t i=0; i<AVBL_SECTORS; ++i){
        	sector_status[i] = sector[i].update(dt_seconds);					//update avbl sectors
    		if (cmd_confirmation_required[i] == true){
    			cmd_confirmation_required[i] = handleConfirmation(sector[i]); 	//reset request flag on success
    		}
    	}


    	if (sector_status_requested[0] or sector_status_requested[1] or sector_status_requested[2] or sector_status_requested[3]){
    		encoded_sectors_status = sector_status[3]<<24 | sector_status[2]<<16 | sector_status[1]<<8 | sector_status[0];
    		xQueueOverwrite(sectorsStatusQueue, &encoded_sectors_status);
    		sector_status_requested[0] = false;
    		sector_status_requested[1] = false;
    		sector_status_requested[2] = false;
    		sector_status_requested[3] = false;
    	}



    	if(xUserButtonSemaphore != NULL){
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE){
			   encoded_sectors_status = sector_status[3]<<24 | sector_status[2]<<16 | sector_status[1]<<8 | sector_status[0];
			   errorcode.reporter = target_t::Sector;
			   errorcode.id = 255;
			   errorcode.code = encoded_sectors_status;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Tank;
			   errorcode.id = tank1->idGet();
			   errorcode.code = tank1_status.state;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   pumpStateEncode(sector[0].irrigationController->p8833Pump->statusGet(), pumps_status);
			   pumpStateEncode(sector[1].irrigationController->p8833Pump->statusGet(), pumps_status);
			   pumpStateEncode(sector[2].irrigationController->p8833Pump->statusGet(), pumps_status);
			   errorcode.reporter = target_t::Pump;
			   errorcode.id = 255;
			   errorcode.code = pumps_status;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Power; //TODO
			   errorcode.id = 255;
			   errorcode.code = 0xffffffff;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::System; //TODO
			   errorcode.id = 255;
			   errorcode.code = 0xffffffff;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

		   }
    	}


    	LEDToggle(10);
		vTaskDelayUntil(&xLastWakeTime, xFrequencySeconds);
    }

    delete tank1;

}
void vStatusNotifyTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();
	servicecode_u rx;

    for( ;; )
    {
		while (xQueueReceive(serviceQueue, &rx.servicecode, ( TickType_t ) 10)){
			HAL_UART_Transmit(&huart4, rx.buffer, 6, 10);
			HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
		}

        vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}
void vWirelessCommTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 0.05 * TASK_FREQ_MULTIPLIER; //<20Hz
	xLastWakeTime=xTaskGetTickCount();

	constexpr struct gpio_s radio1ce = {NRF24_CE_GPIO_Port, NRF24_CE_Pin};
	constexpr struct gpio_s radio1csn = {NRF24_NSS_GPIO_Port, NRF24_NSS_Pin};

	dlframe32byte_u radio1FrameRx;
	struct cmd_s cmd;
	struct confirmation_s confirmation;
	bool radio1_configured = false;
	tankstatus_s tank1_status;
	uint32_t encoded_pumps_status = 0;
	uint32_t encoded_sectors_status = 0;
	struct plant_s plant;
	struct battery_s battery;


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
	/* By default 2Mbps data rate and -6dBm output power */
	/* NRF24L01 goes to RX mode by default */
	radio1->Init(&hspi2,radio1ce, radio1csn);
	while(radio1->Config(PAYLOAD_SIZE, 15, NRF24L01_OutputPower_M6dBm, NRF24L01_DataRate_1M) == false){
		vTaskDelay(500);
	}
	radio1_configured = true;

	/* Set my address, 5 bytes */
	radio1->SetMyAddress(MyAddress);

	/* Set TX address, 5 bytes */
	radio1->SetTxAddress(TxAddress);

	/* Go back to RX mode */
	radio1->PowerUpRx();

    for( ;; )
    {
    	if(radio1_configured == true){

			/* If data is ready on NRF24L01+ */
			while (radio1->DataReady()) {
				/* Get data from NRF24L01+ */
				radio1->GetPayload(radio1FrameRx.buffer);
				if (radio1FrameRx.values.start == direction_t::RPiToIRM)
				{
					IrrigationMessage *inbound_msg = new IrrigationMessage(direction_t::RPiToIRM);
					inbound_msg->setBuffer(radio1FrameRx.buffer, PAYLOAD_SIZE);
					if (inbound_msg->validateCRC() == true){
						cmd = inbound_msg->decodeCommand();
						xQueueSendToFront(extCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
					}

					delete inbound_msg;
				}
			}

			if (xQueueReceive( tanksStatusQueue, &tank1_status, 0 ) == pdPASS){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::IRMToRPi);
				outbound_msg->encode(tank1_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);

			}
			if(xQueueReceive( sectorsStatusQueue, &encoded_sectors_status, 0 ) == pdPASS){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::IRMToRPi);
				outbound_msg->encodeGeneric(target_t::Sector, 255, encoded_sectors_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);
			}

			if(xQueueReceive( pumpsStatusQueue, &encoded_pumps_status, 0 ) == pdPASS){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::IRMToRPi);
				outbound_msg->encodeGeneric(target_t::Pump, 255, encoded_pumps_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);
			}

			while(xQueueReceive( plantsHealthQueue, &plant, 0 )){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::IRMToRPi);
				outbound_msg->encode(plant);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);

			}

			while(xQueueReceive( batteryQueue, &battery, 0 )){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::IRMToRPi);
				//outbound_msg->encode(battery);TODO
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);

			}


			while(xQueueReceive( confirmationsQueue, &confirmation, 0 )){

				IrrigationMessage *outbound_msg = new IrrigationMessage(direction_t::RPiToIRM);
				outbound_msg->encode(confirmation);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
				/* Go back to RX mode */
				radio1->PowerUpRx();
				while(radio1->GetStatus() != 14);
			}
    	}

    	LEDToggle(8);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

    delete radio1;

}

bool handleConfirmation(IrrigationSector &_sector){

	struct confirmation_s confirmation = {target_t::Sector, 0, command_t::None, 0, 0, false};
	bool not_confirmed = true;

	if (_sector.wateringGet() == true and (_sector.pumpstateGet() == pumpstate_t::running or _sector.pumpstateGet() == pumpstate_t::waiting)){
		confirmation.target_id = _sector.sectorGet();
		confirmation.cmd = command_t::Start;
		confirmation.consumed = true;
		not_confirmed = xQueueSendToFront(confirmationsQueue, &confirmation, ( TickType_t ) 0) == pdTRUE ? false : true;

	}
	if (_sector.wateringGet() == false and _sector.pumpstateGet() == pumpstate_t::stopped){
		confirmation.target_id = _sector.sectorGet();
		confirmation.cmd = command_t::Stop;
		confirmation.consumed = true;
		not_confirmed = xQueueSendToFront(confirmationsQueue, &confirmation, ( TickType_t ) 0) == pdTRUE ? false : true;
	}

	return not_confirmed;

}



