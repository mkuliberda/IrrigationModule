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


extern SemaphoreHandle_t xUserButtonSemaphore;
extern SemaphoreHandle_t xADCReadingsReadySemaphore;
extern xQueueHandle ADCValuesQueue;
extern xQueueHandle tank1StatusQueue;
extern xQueueHandle pumpsStatusQueue;
extern xQueueHandle plantsHealthQueue;
extern xQueueHandle batteryStatusQueue;
extern xQueueHandle externalCommandsQueue;
extern xQueueHandle sysStatusQueue;


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
	const portTickType xFrequencySeconds = 0.1 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

	const double tank1HeightMeters = 0.43;
	const double tank1VolumeLiters = 5.0;
	const float WLSensorHighPositionMeters = 0.12;
	const float WLSensorLowPositionMeters = 0.38;

	const struct gpio_s pump1gpio = {PUMP1_GPIO_Port, PUMP1_Pin};
	const struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	const struct gpio_s pump2gpio = {PUMP2_GPIO_Port, PUMP2_Pin};
	const struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	const struct gpio_s pump3gpio = {PUMP3_GPIO_Port, PUMP3_Pin};
	const struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};
	const struct gpio_s ds18b20_1gpio = {DS18B20_1_GPIO_Port, DS18B20_1_Pin};

	const struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	const struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};

	struct plant_s plant1 = {"Pelargonia1", 1, 0.0};
	struct plant_s plant2 = {"Surfinia1", 2, 0.0};
	struct plant_s plant3 = {"Surfinia2", 3, 0.0};
	struct plant_s plant4 = {"Trawa", 4, 0.0};

	double dt_seconds = xFrequencySeconds/1000.0f;

	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0; //8 bits per pump
	uint8_t sectorStatus[3]; //TODO: encode this like pumpStatus?
	uint16_t sector1ADCValue[1] = {1};
	uint16_t sector2ADCValue[2] = {2,3};
	uint16_t sector3ADCValue[1] = {4};
	uint16_t freeADCValue[4] = {5,6,7,8};
	uint16_t batteryADCValue[1] = {9};


	IrrigationSector *sector1 = new IrrigationSector(1);
	sector1->plantCreate(plant1.name, plant1.id);
	sector1->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector1->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector1->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector1->irrigationController->pBinPump->init(1, 4, 10, pump1gpio, pump1led);
	}


	IrrigationSector *sector2 = new IrrigationSector(2);
	sector2->plantCreate(plant2.name, plant2.id);
	sector2->plantCreate(plant3.name, plant3.id);
	sector2->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector2->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector2->irrigationController->pBinPump->init(2, 5, 15, pump2gpio, pump2led);
	}


	IrrigationSector *sector3 = new IrrigationSector(3);
	sector3->plantCreate(plant4.name, plant4.id);
	sector3->irrigationController->modeSet(pumpcontrollermode_t::external);
	sector3->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector3->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector3->irrigationController->pBinPump->init(3, 5, 15, pump3gpio, pump3led);
	}


	WaterTank *tank1 = new WaterTank(tank1HeightMeters, tank1VolumeLiters);
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(0).init(WLSensorHighPositionMeters, opticalwaterlevelsensor1gpio);
	}
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors.at(1).init(WLSensorLowPositionMeters, opticalwaterlevelsensor2gpio);
	}
	if (tank1->temperatureSensorCreate(temperaturesensortype_t::ds18b20) == true){
		tank1->vTemperatureSensors.at(0).init(ds18b20_1gpio, &htim7);
	}


	bool test_cmd1 = true;
	bool test_cmd2 = true;
	bool test_cmd3 = true;
	bool planthealth_req = true;

    for( ;; )
    {
    	dt_seconds = xFrequencySeconds/1000.0f;
    	tank1->checkStateOK(dt_seconds, tank1Status); //TODO: first check state and base pump running if state OK, there is a BUG here
    	xQueueOverwrite( tank1StatusQueue, &tank1Status);

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

		sector1->update(dt_seconds, test_cmd1, sector1ADCValue, 1);
		sector2->update(dt_seconds, test_cmd2, sector2ADCValue, 2);
		sector3->update(dt_seconds, test_cmd3, sector3ADCValue, 1);

		if(planthealth_req) plant1.health = sector1->planthealthGet(plant1.id);
		if(planthealth_req) plant2.health = sector2->planthealthGet(plant2.id);
		if(planthealth_req) plant3.health = sector2->planthealthGet(plant3.id);
		if(planthealth_req) plant4.health = sector3->planthealthGet(plant4.id);


	   	pumpStateEncode(sector1->irrigationController->pBinPump->statusGet(), pumpsStatus);
	    pumpStateEncode(sector2->irrigationController->pBinPump->statusGet(), pumpsStatus);
	    pumpStateEncode(sector3->irrigationController->pBinPump->statusGet(), pumpsStatus);
	    xQueueOverwrite( pumpsStatusQueue, &pumpsStatus);


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
	USART_Buffer32 tank1Status;
	USART_Buffer32 pumpsStatus;
	array<struct pumpstatus_s,4> a_pumpStatus;



    for( ;; )
    {

    	if(xUserButtonSemaphore != NULL)
    	{
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE)
		   {
			   if (xQueuePeek(pumpsStatusQueue, &pumpsStatus.status, 0) == pdPASS)
			   {
				   pumpStateDecode(a_pumpStatus, pumpsStatus.status);
				   HAL_UART_Transmit(&huart4, pumpsStatus.buffer, 4, 50);
			   }
			   if (xQueuePeek( tank1StatusQueue, &tank1Status.status, 0 ) == pdPASS)
			   {
				   HAL_UART_Transmit(&huart4, tank1Status.buffer, 4, 50);
			   }
			   HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, GPIO_PIN_RESET);
		   }
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

	radio1FrameTx.values.start = commdirection_t::irm_to_rpi;
	radio1FrameTx.values.target = 0;
	radio1FrameTx.values.id = 0;
	radio1FrameTx.values.val = 0.0;
	//radio1FrameTx.values.desc = "empty description"; TODO
	radio1FrameTx.values.crc8 = 0;

	//radio1FrameRx.values.start = 0;
	//radio1FrameRx.values.target = 0;
	radio1FrameRx.values.id = 0;
	//radio1FrameRx.values.cmd = 0;
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
			if (radio1->DataReady()) {
				/* Get data from NRF24L01+ */
				radio1->GetPayload(radio1FrameRx.buffer);
				if (radio1FrameRx.values.start == commdirection_t::rpi_to_irm) //TODO: add crc
				{
					// Post received message.
					cmd.target = radio1FrameRx.values.target;
					cmd.id = radio1FrameRx.values.id;
					cmd.cmd = radio1FrameRx.values.cmd;
					cmd.subcmd1 = radio1FrameRx.values.subcmd1;
					cmd.subcmd1 = radio1FrameRx.values.subcmd2;
					cmd.subcmd1 = radio1FrameRx.values.subcmd3;
					cmd.subcmd1 = radio1FrameRx.values.subcmd4;
					xQueueSendToFront(externalCommandsQueue, &cmd, ( TickType_t ) 0);
				}
			}

			//if(TODO){
			radio1->TransmitPayload(radio1FrameTx.buffer);
			//}

			/* Wait for data to be sent */
			do {
				/* Wait till sending */
				transmissionStatus = radio1->GetTransmissionStatus();
			} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);

			/* Go back to RX mode */
			radio1->PowerUpRx();
    	}

    	LEDToggle(7);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

    delete radio1;

}



