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
#include <cstring>
#include "main.h"
#include "adc.h"

#define TANK1STATUS_BUFFER_LENGTH 1
#define PUMPSSTATUS_BUFFER_LENGTH 1
#define SOILMOISTURE_BUFFER_LENGTH 8
#define BATTERY_BUFFER_LENGTH 1


SemaphoreHandle_t xUserButtonSemaphore = NULL;
xQueueHandle tank1StatusQueue;
xQueueHandle pumpsStatusQueue;
xQueueHandle soilMoistureQueue;
xQueueHandle batteryQueue;

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
	portTickType xLastWakeTime;
	const portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER; //<1Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	LEDToggle(10);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }
}

void vUserButtonCheckTask(void *pvParameters )
{
	vSemaphoreCreateBinary(xUserButtonSemaphore);

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
	const struct gpio_s ds18b20gpio = {ONEWIRE_GPIO_Port, ONEWIRE_Pin};

	const struct gpio_s opticalwaterlevelsensor1gpio = {T1_WATER_LVL_H_GPIO_Port, T1_WATER_LVL_H_Pin};
	const struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};

	double dt_seconds = xFrequencySeconds/1000.0f;

	uint32_t tank1Status = 0;
	uint32_t pumpsStatus = 0; //8 bits per pump
	uint16_t adcValues[9] = {1,2,3,4,5,6,7,8,9};

	tank1StatusQueue = xQueueCreate(TANK1STATUS_BUFFER_LENGTH, sizeof( uint32_t ) );
	pumpsStatusQueue = xQueueCreate(PUMPSSTATUS_BUFFER_LENGTH, sizeof( uint32_t ) );
	ADCValuesQueue = xQueueCreate(ADCVALUES_BUFFER_LENGTH, sizeof( uint16_t ) );
	vSemaphoreCreateBinary(xADCReadingsReadySemaphore);



	IrrigationSector *sector1 = new IrrigationSector(1);
	sector1->plantCreate("Pelargonia");
	sector1->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector1->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector1->irrigationController->pBinPump->init(1, 2, 5, pump1gpio, pump1led);
	}


	IrrigationSector *sector2 = new IrrigationSector(2);
	sector2->plantCreate("Surfinia1");
	sector2->plantCreate("Surfinia2");
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector2->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector2->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector2->irrigationController->pBinPump->init(2, 3, 10, pump2gpio, pump2led);
	}


	IrrigationSector *sector3 = new IrrigationSector(3);
	sector3->plantCreate("Trawa");
	sector3->irrigationController->moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector3->irrigationController->pumpCreate(pumptype_t::binary) == true){
		sector3->irrigationController->pBinPump->init(3, 3, 10, pump3gpio, pump3led);
	}


	WaterTank *tank1 = new WaterTank(tank1HeightMeters, tank1VolumeLiters);
	tank1->init();
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors[0].init(WLSensorHighPositionMeters, opticalwaterlevelsensor1gpio);
	}
	if (tank1->waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1->vOpticalWLSensors[1].init(WLSensorLowPositionMeters, opticalwaterlevelsensor2gpio);
	}
	if (tank1->temperatureSensorCreate(temperaturesensortype_t::ds18b20) == true){
		tank1->vTemperatureSensors[0].init(ds18b20gpio, &htim7);
	}


	bool test_cmd = true;
	bool cmd_consumed = false;
	uint32_t test_cnt = 0;

    for( ;; )
    {

    	tank1->checkStateOK(tank1Status);
    	tank1Status+=test_cnt;
    	xQueueOverwrite( tank1StatusQueue, &tank1Status);

    	pumpStateEncode(sector1->irrigationController->pBinPump->status, pumpsStatus);
    	pumpStateEncode(sector2->irrigationController->pBinPump->status, pumpsStatus);
    	pumpStateEncode(sector3->irrigationController->pBinPump->status, pumpsStatus);
    	xQueueOverwrite( pumpsStatusQueue, &pumpsStatus);

    	if(xADCReadingsReadySemaphore != NULL)
    	{
		   if (xSemaphoreTake(xADCReadingsReadySemaphore, (portTickType)2) == pdTRUE)
		   {
				for (uint8_t i=0; i<9;i++) xQueueReceive(ADCValuesQueue, &adcValues[i], 0);
				sector1->irrigationController->vDMAMoistureSensor[0].rawUpdate(adcValues[0]);
				sector2->irrigationController->vDMAMoistureSensor[0].rawUpdate(adcValues[1]);
				sector2->irrigationController->vDMAMoistureSensor[1].rawUpdate(adcValues[2]);
				sector3->irrigationController->vDMAMoistureSensor[0].rawUpdate(adcValues[3]);
		   }
    	}
    	//-----------test/development part only, to be deleted in final version----------
    	if(test_cnt < 200)
    	{
    		test_cmd = true;
    		sector1->irrigationController->pBinPump->run(dt_seconds,test_cmd, cmd_consumed);
    	}
    	else if(test_cnt >= 200 && test_cnt < 400)
    	{
    		if(test_cnt == 250) cmd_consumed = false;
    		test_cmd = false;
    		sector1->irrigationController->pBinPump->run(dt_seconds,test_cmd, cmd_consumed);
    	}
    	else if(test_cnt >= 400)
		{
    		cmd_consumed = false;
    		test_cnt = 0;
		}
    	test_cnt++;
    	//------------------------------------------------------------------------------




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
	const portTickType xFrequencySeconds = 0.05 * TASK_FREQ_MULTIPLIER; //<10Hz
	xLastWakeTime=xTaskGetTickCount();

    for( ;; )
    {
    	LEDToggle(7);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }

}



