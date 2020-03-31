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
extern xQueueHandle adcValuesQueue;
extern xQueueHandle tanksStatusQueue;
extern xQueueHandle pumpsStatusQueue;
extern xQueueHandle sectorsStatusQueue;
extern xQueueHandle plantsHealthQueue;
extern xQueueHandle batteryStatusQueue;
extern xQueueHandle extCommandsQueue;
extern xQueueHandle sysStatusQueue;
extern xQueueHandle serviceQueue;


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
    	LEDToggle(10);
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

	constexpr double tank1_height_meters = 0.43;
	constexpr double tank1_volume_liters = 5.0;
	constexpr float wls_high_pos_meters = 0.12;
	constexpr float wls_low_pos_meters = 0.38;

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

	struct plant_s plant1 = {"Pelargonia1", PLANT1_ID, 0.0};
	struct plant_s plant2 = {"Surfinia1", PLANT2_ID, 0.0};
	struct plant_s plant3 = {"Surfinia2", PLANT3_ID, 0.0};
	struct plant_s plant4 = {"Trawa", PLANT4_ID, 0.0};

	struct servicecode_s errorcode;

	uint8_t rcvd_cmds_nbr = 0;

	double dt_seconds = xFrequencySeconds/1000.0f;

	struct tankstatus_s tank1_status = {1, 0};
	uint32_t pumps_status = 0; //8 bits per pump
	uint32_t sectors_status = 0;
	uint8_t sector_status[MAX_ENTITIES];
	bool sector_status_requested[MAX_ENTITIES];
	uint16_t sector1_adc_value[1] = {1};
	uint16_t sector2_adc_value[2] = {2,3};
	uint16_t sector3_adc_value[1] = {4};
	uint16_t free_adc_value[4] = {5,6,7,8};
	uint16_t battery_adc_value[1] = {9};


	IrrigationSector *sector1 = new IrrigationSector(SECTOR1_ID);
	sector1->plantCreate(plant1.name, plant1.id);
	sector1->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector1->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector1->irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector1->irrigationController->p8833Pump->init(1, 4, 10, pump1gpio, pump1led, pump1fault, pump1mode);
	}


	IrrigationSector *sector2 = new IrrigationSector(SECTOR2_ID);
	sector2->plantCreate(plant2.name, plant2.id);
	sector2->plantCreate(plant3.name, plant3.id);
	sector2->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector2->irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector2->irrigationController->p8833Pump->init(2, 5, 15, pump2gpio, pump2led, pump2fault, pump2mode);
	}


	IrrigationSector *sector3 = new IrrigationSector(SECTOR3_ID);
	sector3->plantCreate(plant4.name, plant4.id);
	sector3->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector3->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector3->irrigationController->pumpCreate(pumptype_t::drv8833_dc) == true){
		sector3->irrigationController->p8833Pump->init(3, 7, 18, pump3gpio, pump3led, pump3fault, pump3mode);
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


	bool sector1_irrigate = false; //TODO: false
	bool sector2_irrigate = false; //TODO: false
	bool sector3_irrigate = false; //TODO: false
	bool watertank1_valid = false;



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
		   }
    	}


		struct cmd_s received_commands[EXTCMDS_BUFFER_LENGTH];
		struct cmd_s rx;
    	while (xQueueReceive(extCommandsQueue, &rx, ( TickType_t ) 10)){
    		received_commands[rcvd_cmds_nbr] = rx;
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
				   	pumpStateEncode(sector1->irrigationController->p8833Pump->statusGet(), pumps_status);
				    pumpStateEncode(sector2->irrigationController->p8833Pump->statusGet(), pumps_status);
				    pumpStateEncode(sector3->irrigationController->p8833Pump->statusGet(), pumps_status);
				    xQueueOverwrite( pumpsStatusQueue, &pumps_status);
					break;

				case target_t::Plant:
					if		(received_commands[i].target_id == plant1.id){
						plant1.health = sector1->planthealthGet(plant1.id);
						xQueueSendToFront(plantsHealthQueue, &plant1.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant2.id){
						plant2.health = sector2->planthealthGet(plant2.id);
						xQueueSendToFront(plantsHealthQueue, &plant2.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant3.id){
						plant3.health = sector2->planthealthGet(plant3.id);
						xQueueSendToFront(plantsHealthQueue, &plant3.health, ( TickType_t ) 0);
					}
					else if	(received_commands[i].target_id == plant4.id){
						plant4.health = sector3->planthealthGet(plant4.id);
						xQueueSendToFront(plantsHealthQueue, &plant4.health, ( TickType_t ) 0);
					}
					else if (received_commands[i].target_id == 255){
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

				case target_t::Power:
					break;

				case target_t::System:
					break;

				case target_t::Sector:
					if		(received_commands[i].target_id == sector1->sectorGet()){
						sector_status_requested[0] = true;
					}
					else if	(received_commands[i].target_id == sector2->sectorGet()){
						sector_status_requested[1] = true;
					}
					else if	(received_commands[i].target_id == sector3->sectorGet()){
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
					HAL_GPIO_TogglePin(LD8_GPIO_Port, LD8_Pin);
					break;

				default:
					break;
			}
			}
			else if(received_commands[i].cmd == command_t::Start){

				if (received_commands[i].target == target_t::Sector){

					switch (received_commands[i].target_id){
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
			}
			else if(received_commands[i].cmd == command_t::Stop){

				if (received_commands[i].target == target_t::Sector){

					switch (received_commands[i].target_id){
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

    	sector_status[0] = sector1->update(dt_seconds, sector1_irrigate, sector1_adc_value, 1);
    	sector_status[1] = sector2->update(dt_seconds, sector2_irrigate, sector2_adc_value, 2);
    	sector_status[2] = sector3->update(dt_seconds, sector3_irrigate, sector3_adc_value, 1);
    	sectors_status = sector_status[3]<<24 | sector_status[2]<<16 | sector_status[1]<<8 | sector_status[0];


    	if (sector_status_requested[0] or sector_status_requested[1] or sector_status_requested[2] or sector_status_requested[3]){
    		xQueueOverwrite(sectorsStatusQueue, &sectors_status);
    		sector_status_requested[0] = false;
    		sector_status_requested[1] = false;
    		sector_status_requested[2] = false;
    		sector_status_requested[3] = false;
    	}


    	if(xUserButtonSemaphore != NULL){
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE){
			   errorcode.reporter = target_t::Sector;
			   errorcode.id = 255;
			   errorcode.code = sectors_status;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Tank;
			   errorcode.id = tank1->idGet();
			   errorcode.code = tank1_status.state;
			   xQueueSendToFront(serviceQueue, &errorcode, ( TickType_t ) 0);

			   pumpStateEncode(sector1->irrigationController->p8833Pump->statusGet(), pumps_status);
			   pumpStateEncode(sector2->irrigationController->p8833Pump->statusGet(), pumps_status);
			   pumpStateEncode(sector3->irrigationController->p8833Pump->statusGet(), pumps_status);
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


    	LEDToggle(6);
		vTaskDelayUntil(&xLastWakeTime, xFrequencySeconds);
    }

    delete sector1; delete sector2; delete sector3;
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
			HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_RESET);
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
	bool radio1_configured = false;
	tankstatus_s tank1_status;
	uint32_t pumps_status = 0;
	uint32_t sectors_status = 0;
	struct plant_s plant;

	//-------------------------

	//radio1FrameTx.values.start = direction_t::IRMToRPi;

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
	radio1_configured = radio1->Config(PAYLOAD_SIZE, 15, NRF24L01_OutputPower_M6dBm, NRF24L01_DataRate_2M);

	/* Set my address, 5 bytes */
	radio1->SetMyAddress(MyAddress);

	/* Set TX address, 5 bytes */
	radio1->SetTxAddress(TxAddress);

    for( ;; )
    {
    	if(radio1_configured == true){

			/* If data is ready on NRF24L01+ */
			while (radio1->DataReady()) {
				/* Get data from NRF24L01+ */
				radio1->GetPayload(radio1FrameRx.buffer);
				if (radio1FrameRx.values.start == direction_t::RPiToIRM)
				{
					IrrigationMessage32 *inbound_msg = new IrrigationMessage32(direction_t::RPiToIRM);
					inbound_msg->setBuffer(radio1FrameRx.buffer, PAYLOAD_SIZE);
					if (inbound_msg->validateCRC() == true){
						cmd = inbound_msg->decodeCommand();
						xQueueSendToFront(extCommandsQueue, (void *)&cmd, ( TickType_t ) 0);
					}

					delete inbound_msg;
				}
			}

			if (xQueueReceive( tanksStatusQueue, &tank1_status, 0 ) == pdPASS){

				IrrigationMessage32 *outbound_msg = new IrrigationMessage32(direction_t::IRMToRPi);
				outbound_msg->encode(tank1_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;

			}
			else if(xQueueReceive( sectorsStatusQueue, &sectors_status, 0 ) == pdPASS){

				IrrigationMessage32 *outbound_msg = new IrrigationMessage32(direction_t::IRMToRPi);
				outbound_msg->encodeGeneric(target_t::Sector, 255, sectors_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
			}

			else if(xQueueReceive( pumpsStatusQueue, &pumps_status, 0 ) == pdPASS){

				IrrigationMessage32 *outbound_msg = new IrrigationMessage32(direction_t::IRMToRPi);
				outbound_msg->encodeGeneric(target_t::Pump, 255, pumps_status);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;
			}

			while(xQueueReceive( plantsHealthQueue, &plant, 0 )){

				IrrigationMessage32 *outbound_msg = new IrrigationMessage32(direction_t::IRMToRPi);
				outbound_msg->encode(plant);
				radio1->TransmitPayload(outbound_msg->uplinkframe.buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1->GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

				delete outbound_msg;

			}

			/* Go back to RX mode */
			radio1->PowerUpRx();

    	}

    	LEDToggle(7);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

    delete radio1;

}



