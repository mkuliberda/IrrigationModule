/*
 * freeRTOSTasks.cpp
 *
 *  Created on: 30.11.2019
 *      Author: Mati
 */

#include <freeRTOSTasks.h>
#include <irrigation.h>
#include <utilities.h>
#include <plants.h>
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "iwdg.h"
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
#define SECTOR4_ID 3
#define WATERTANK1_ID 0
#define AVBL_SECTORS 1
#define BATTERY1_ID 0

#define REFRESH_RATE_SECONDS_ANALOG 2
#define REFRESH_RATE_SECONDS_OTHER 0.5
#define REFRESH_RATE_SECONDS_IRRIGATION 0.5
#define REFRESH_RATE_SECONDS_COMMS 0.02


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

IWDG_HandleTypeDef hiwdg;

using namespace std;

void vADCReadTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = REFRESH_RATE_SECONDS_ANALOG * TASK_FREQ_MULTIPLIER;
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
	constexpr portTickType xFrequencySeconds = REFRESH_RATE_SECONDS_OTHER * TASK_FREQ_MULTIPLIER;
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
	constexpr portTickType xFrequencySeconds = REFRESH_RATE_SECONDS_OTHER * TASK_FREQ_MULTIPLIER;
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
	constexpr portTickType xFrequencySeconds = REFRESH_RATE_SECONDS_IRRIGATION * TASK_FREQ_MULTIPLIER;
	xLastWakeTime=xTaskGetTickCount();

	constexpr double tank1_height_meters = 0.55;
	constexpr double tank1_volume_liters = 50.0;
	constexpr float wls_low_pos_meters = 0.0825;
	constexpr uint32_t pump_maxruntime_seconds = 1800;
	constexpr uint32_t pump_breaktime_seconds = 60;
	constexpr float battery1_capacity = 4400;
	constexpr float battery1_cell_count = 1;
	constexpr float battery1_volt_div_error_factor = 1.52378;
	constexpr float adc_reference_voltage = 3.3;
	constexpr uint32_t adc_voltage_levels = 4095;

	/*constexpr struct gpio_s pump1gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP1_IN1_Pin};
	constexpr struct gpio_s pump1gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP1_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump1gpio = {pump1gpio_in1, pump1gpio_in2};
	constexpr struct gpio_s pump1led  = {PUMP1LD_GPIO_Port, PUMP1LD_Pin};
	constexpr struct gpio_s pump1fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_ULT_Pin};
	constexpr struct gpio_s pump1mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_EEP_Pin};*/

	constexpr struct gpio_s pump2gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP2_IN1_Pin};
	constexpr struct gpio_s pump2gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP2_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump2gpio = {pump2gpio_in1, pump2gpio_in2};
	constexpr struct gpio_s pump2led  = {PUMP2LD_GPIO_Port, PUMP2LD_Pin};
	constexpr struct gpio_s pump2fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_ULT_Pin};
	constexpr struct gpio_s pump2mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_1_EEP_Pin};

	/*constexpr struct gpio_s pump3gpio_in1 = {DRV8833PUMPS_GPIO_Port, PUMP3_IN1_Pin};
	constexpr struct gpio_s pump3gpio_in2 = {DRV8833PUMPS_GPIO_Port, PUMP3_IN2_Pin};
	constexpr array<struct gpio_s, 2> pump3gpio = {pump3gpio_in1, pump3gpio_in2};
	constexpr struct gpio_s pump3led  = {PUMP3LD_GPIO_Port, PUMP3LD_Pin};
	constexpr struct gpio_s pump3fault  = {DRV8833PUMPS_GPIO_Port, DRV8833_2_ULT_Pin};
	constexpr struct gpio_s pump3mode  = {DRV8833PUMPS_GPIO_Port, DRV8833_2_EEP_Pin};*/

	constexpr struct gpio_s ds18b20_1gpio = {DS18B20_1_GPIO_Port, DS18B20_1_Pin};
	constexpr struct gpio_s opticalwaterlevelsensor2gpio = {T1_WATER_LVL_L_GPIO_Port, T1_WATER_LVL_L_Pin};

	struct servicecode_s errorcode;
	struct batterystatus_s battery_info;
	struct sectorstatus_s sector_info;
	struct plantstatus_s plant_info;
	struct tankstatus_s watertank_info = {WATERTANK1_ID, 0};

	struct cmd_s received_commands[EXTCMDS_BUFFER_LENGTH];
	uint8_t rcvd_cmds_nbr = 0;

	double dt_seconds = xFrequencySeconds/1000.0f;

	uint32_t encoded_pumps_status = 0; //8 bits per pump
	uint8_t sector_status[MAX_ENTITIES] = {0, 0, 0, 0};
	bool sector_status_requested[MAX_ENTITIES] =  {false, false, false, false};
	bool cmd_confirmation_required[MAX_ENTITIES] = {false, false, false, false};
	uint16_t sector1_adc_value[3] = {1,2,3};
	uint16_t free_adc_value[5] = {4,5,6,7,8};
	uint16_t battery_adc_value[1] = {9};
	bool watertank1_valid = false;
	IrrigationSector sector[AVBL_SECTORS] = {{SECTOR1_ID}};

	WaterTank tank1(tank1_height_meters, tank1_volume_liters, WATERTANK1_ID);
	if (tank1.waterlevelSensorCreate(waterlevelsensortype_t::optical) == true){
		tank1.vOpticalWLSensors.at(0).init(wls_low_pos_meters, opticalwaterlevelsensor2gpio);
	}

	if (tank1.temperatureSensorCreate(temperaturesensortype_t::ds18b20) == true){
		tank1.vTemperatureSensors.at(0).init(ds18b20_1gpio, &htim7);
	}


	/*sector[0].createPlant("ch1", PLANT1_ID);
	sector[0].createPlant("ch5", PLANT5_ID);
	sector[0].irrigationController.modeSet(pumpcontrollermode_t::external);
	sector[0].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector[0].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[0].irrigationController.pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[0].irrigationController.p8833Pump->init(PUMP1_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump1gpio, pump1led, pump1fault, pump1mode);
	}


	sector[1].createPlant("ch2", PLANT2_ID);
	sector[1].createPlant("ch3", PLANT3_ID);
	sector[1].irrigationController.modeSet(pumpcontrollermode_t::external);
	sector[1].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector[1].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[1].irrigationController.pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[1].irrigationController.p8833Pump->init(PUMP2_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump2gpio, pump2led, pump2fault, pump2mode);
	}*/


	sector[0].createPlant("ch1", PLANT1_ID);
	sector[0].createPlant("ch2", PLANT2_ID);
	sector[0].createPlant("ch3", PLANT3_ID);
	sector[0].irrigationController.modeSet(pumpcontrollermode_t::external);
	sector[0].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector[0].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	sector[0].irrigationController.moisturesensorCreate(moisturesensortype_t::capacitive_noshield);
	if (sector[0].irrigationController.pumpCreate(pumptype_t::drv8833_dc) == true){
		sector[0].irrigationController.p8833Pump->init(PUMP2_ID, pump_breaktime_seconds, pump_maxruntime_seconds, pump2gpio, pump2led, pump2fault, pump2mode);
	}

	Battery battery1(BATTERY1_ID, batterytype_t::liion, batteryinterface_t::adc, battery1_cell_count, battery1_capacity);
	battery1.configureAdcCharacteristics(battery1_volt_div_error_factor, adc_reference_voltage, adc_voltage_levels);


    for( ;; )
    {
    	HAL_IWDG_Refresh(&hiwdg);
    	//TODO calculate dt_seconds based on real world period instead of a fixed one
    	rcvd_cmds_nbr = 0;
    	watertank1_valid = tank1.checkStateOK(dt_seconds, watertank_info.state);

    	if(xADCReadingsReadySemaphore != NULL)
    	{
		   if (xSemaphoreTake(xADCReadingsReadySemaphore, (portTickType)2) == pdTRUE)
		   {
				for (uint8_t i=0; i<9;++i){
					switch (i)
					{
					case 0:
						xQueueReceive(adcValuesQueue, &battery_adc_value[0], 0);
						break;
					case 1:
						xQueueReceive(adcValuesQueue, &sector1_adc_value[0], 0); //2
						break;
					case 2:
						xQueueReceive(adcValuesQueue, &sector1_adc_value[1], 0); //1
						break;
					case 3:
						xQueueReceive(adcValuesQueue, &sector1_adc_value[2], 0); //3
						break;
					case 4:
						xQueueReceive(adcValuesQueue, &free_adc_value[3], 0); //4
						break;
					case 5:
						xQueueReceive(adcValuesQueue, &free_adc_value[4], 0); //5
						break;
					case 6:
						xQueueReceive(adcValuesQueue, &free_adc_value[0], 0); //6
						break;
					case 7:
						xQueueReceive(adcValuesQueue, &free_adc_value[2], 0); //8
						break;
					case 8:
						xQueueReceive(adcValuesQueue, &free_adc_value[1], 0); //7
						break;
					default:
						break;
					}
				}
				//sector[0].setMeasurements(sector1_adc_value, 2);
				//sector[1].setMeasurements(sector2_adc_value, 2);
				sector[0].setMeasurements(sector1_adc_value, 3);
				battery1.update(REFRESH_RATE_SECONDS_ANALOG, battery_adc_value, 1);
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
					xQueueOverwrite( tanksStatusQueue, &watertank_info);
					break;

				case target_t::Pump:
					break;

				case target_t::Plant:
					if (received_commands[i].target_id != 255){
					for (uint8_t sector_id = 0; sector_id < AVBL_SECTORS; ++sector_id){
						if (sector[sector_id].getPlantHealth(received_commands[i].target_id) != -1000){
							plant_info.id = received_commands[i].target_id;
							sector[sector_id].getPlantNameByID(received_commands[i].target_id).copy(plant_info.name, NAME_LENGTH);
							plant_info.health = sector[sector_id].getPlantHealth(received_commands[i].target_id);
							xQueueSendToFront(plantsHealthQueue, (void *)&plant_info, ( TickType_t ) 10);
						}
					}
					}
					else {
						for (uint8_t sector_id = 0; sector_id < AVBL_SECTORS; ++sector_id){
							for (uint8_t plant_id = 0; plant_id < PLANTSHEALTH_BUFFER_LENGTH; ++plant_id ){
								plant_info.health = sector[sector_id].getPlantHealth(plant_id);
								if (plant_info.health != -1000){
									plant_info.id = plant_id;
									sector[sector_id].getPlantNameByID(plant_id).copy(plant_info.name, NAME_LENGTH);
									xQueueSendToFront(plantsHealthQueue, (void *)&plant_info, ( TickType_t ) 10);
								}
							}
						}
					}
					break;

				case target_t::Power:
					if (received_commands[i].target_id == battery_info.id){
						battery_info.percentage = battery1.getPercentage();
						battery_info.status = battery1.getStatus();
						battery_info.remaining_time_min = battery1.getRemainingTimeMinutes();
						xQueueOverwrite( batteryStatusQueue, (void *)&battery_info);
					}
					break;

				case target_t::System:
					break;

				case target_t::Sector:
					if		(received_commands[i].target_id == sector[0].getSector()){
						sector_status_requested[0] = true;
					}
					/*else if	(received_commands[i].target_id == sector[1].getSector()){
						sector_status_requested[1] = true;
					}
					else if	(received_commands[i].target_id == sector[2].getSector()){
						sector_status_requested[2] = true;
					}
					else if	(received_commands[i].target_id == sector[3].getSector()){
						sector_status_requested[3] = true;
					}*/
					else if	(received_commands[i].target_id == 255){
						sector_status_requested[0] = true;
						sector_status_requested[1] = true;
						sector_status_requested[2] = true;
						sector_status_requested[3] = true;
					}
					break;

				case target_t::All:
					//Plants
					for (uint8_t sector_id = 0; sector_id < AVBL_SECTORS; ++sector_id){
						for (uint8_t plant_id = 0; plant_id < PLANTSHEALTH_BUFFER_LENGTH; ++plant_id ){
							plant_info.health = sector[sector_id].getPlantHealth(plant_id);
							if (plant_info.health != -1000){
								plant_info.id = plant_id;
								sector[sector_id].getPlantNameByID(plant_id).copy(plant_info.name, NAME_LENGTH);
								xQueueSendToFront(plantsHealthQueue, (void *)&plant_info, ( TickType_t ) 10);
							}
						}
					}
					// Report tanks status
					xQueueOverwrite( tanksStatusQueue, (void *)&watertank_info);
					// Report batteries status
					battery_info.id = battery1.getId();
					battery_info.percentage = battery1.getPercentage();
					battery_info.status = battery1.getStatus();
					battery_info.remaining_time_min = battery1.getRemainingTimeMinutes();
					xQueueOverwrite( batteryStatusQueue, (void *)&battery_info);
					// Report sectors status
					sector_status_requested[0] = true;
					sector_status_requested[1] = true;
					sector_status_requested[2] = true;
					sector_status_requested[3] = true;
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
						if (watertank1_valid and battery1.isValid()){
							sector[0].wateringSet(true);
							cmd_confirmation_required[0] = true;
						}
						else sector[0].wateringSet(false);
						break;

					case SECTOR2_ID:
						/*if (watertank1_valid and battery1.isValid()){
							sector[1].wateringSet(true);
							cmd_confirmation_required[1] = true;
						}
						else sector[1].wateringSet(false);*/
						break;

					case SECTOR3_ID:
						/*if (watertank1_valid and battery1.isValid()){
							sector[2].wateringSet(true);
							cmd_confirmation_required[2] = true;
						}
						else sector[2].wateringSet(false);*/
						break;

					case SECTOR4_ID:
						/*if (watertank1_valid and battery1.isValid()){
							sector[3].wateringSet(true);
							cmd_confirmation_required[3] = true;
						}
						else sector[3].wateringSet(false);*/
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
						/*sector[1].wateringSet(false);
						cmd_confirmation_required[1] = true;*/
						break;

					case SECTOR3_ID:
						/*sector[0].wateringSet(false);
						cmd_confirmation_required[2] = true;*/
						break;

					case SECTOR4_ID:
						/*sector[3].wateringSet(false);
						cmd_confirmation_required[3] = true;*/
						break;

					default:
						break;
					}
				}
			}

    	}


    	for (uint8_t i=0; i<AVBL_SECTORS; ++i){
    		if (watertank1_valid == false or battery1.isValid() == false) sector[i].wateringSet(false);
        	sector_status[i] = sector[i].update(dt_seconds);					//update avbl sectors
    		if (cmd_confirmation_required[i] == true){
    			cmd_confirmation_required[i] = sector[i].handleConfirmation(); 	//reset request flag on success
    		}
    		if(sector_status_requested[i]){
        		sector_status_requested[i] = false;
    			sector_info = sector[i].getInfo();
    			xQueueSendToFront(sectorsStatusQueue, (void *)&sector_info, ( TickType_t ) 0);
    		}
    	}


    	if(xUserButtonSemaphore != NULL){
		   if (xSemaphoreTake(xUserButtonSemaphore, (portTickType)2) == pdTRUE){
			   errorcode.reporter = target_t::Sector;
			   errorcode.id = 255;
			   errorcode.code = sector_status[3]<<24 | sector_status[2]<<16 | sector_status[1]<<8 | sector_status[0];
			   xQueueSendToFront(serviceQueue, (void *)&errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Tank;
			   errorcode.id = tank1.idGet();
			   errorcode.code = watertank_info.state;
			   xQueueSendToFront(serviceQueue, (void *)&errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Pump;
			   errorcode.id = 255;
			   errorcode.code = encoded_pumps_status;
			   xQueueSendToFront(serviceQueue, (void *)&errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::Power;
			   errorcode.id = battery1.getId();
			   errorcode.code = battery1.getStatus();
			   xQueueSendToFront(serviceQueue, (void *)&errorcode, ( TickType_t ) 0);

			   errorcode.reporter = target_t::System; //TODO
			   errorcode.id = 255;
			   errorcode.code = 0xffffffff;
			   xQueueSendToFront(serviceQueue, (void *)&errorcode, ( TickType_t ) 0);

		   }
    	}

    	LEDToggle(10);
		vTaskDelayUntil(&xLastWakeTime, xFrequencySeconds);
    }

}
void vStatusNotifyTask( void *pvParameters )
{
	portTickType xLastWakeTime;
	constexpr portTickType xFrequencySeconds = 1 * TASK_FREQ_MULTIPLIER;
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
	constexpr portTickType xFrequencySeconds = REFRESH_RATE_SECONDS_COMMS * TASK_FREQ_MULTIPLIER;
	xLastWakeTime=xTaskGetTickCount();

	constexpr struct gpio_s radio1ce = {NRF24_CE_GPIO_Port, NRF24_CE_Pin};
	constexpr struct gpio_s radio1csn = {NRF24_NSS_GPIO_Port, NRF24_NSS_Pin};

	bool radio1_configured = false;

	struct cmd_s cmd;
	struct confirmation_s confirmation;
	struct tankstatus_s watertank;
	struct sectorstatus_s sector_info;
	uint32_t encoded_pumps_status = 0;
	struct plantstatus_s plant_info;
	struct batterystatus_s battery_info;

	IrrigationMessage outbound_msg(direction_t::IRMToRPi);
	IrrigationMessage inbound_msg(direction_t::RPiToIRM);


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

	NRF24L01 radio1;

	/* Initialize NRF24L01+ on channel 1 and 32bytes of payload */
	/* By default 250kbps data rate and -12dBm output power */
	/* NRF24L01 goes to RX mode by default */
	radio1.Init(&hspi2, radio1ce, radio1csn);
	while(radio1.Config(PAYLOAD_SIZE, 1, NRF24L01_OutputPower_M12dBm, NRF24L01_DataRate_250k) == false){
		vTaskDelay(500);
	}
	radio1_configured = true;

	/* Set my address, 5 bytes */
	radio1.SetMyAddress(MyAddress);

	/* Set TX address, 5 bytes */
	radio1.SetTxAddress(TxAddress);

	/* Go back to RX mode */
	radio1.PowerUpRx();


    for( ;; )
    {
    	if(radio1_configured == true){

    		if (radio1.isTransmitter()){
    			radio1.PowerUpRx();
    			while(radio1.GetStatus() != 14);
    		}

			/* If data is ready on NRF24L01+ */
			while (radio1.DataReady()) {
				//radio1_fail_counter = 0.0;
				/* Get data from NRF24L01+ */
				dlframe32byte_u radio1FrameRx;
				radio1.GetPayload(radio1FrameRx.buffer);
				if (radio1FrameRx.values.start == direction_t::RPiToIRM)
				{
					inbound_msg.setBuffer(radio1FrameRx.buffer, PAYLOAD_SIZE);
					if (inbound_msg.validateCRC() == true){
						cmd = inbound_msg.decodeCommand();
						xQueueSendToFront(extCommandsQueue, (void *)&cmd, ( TickType_t ) 1);
					}
				}
			}

			while(xQueueReceive( sectorsStatusQueue, &sector_info, 1 )){

				outbound_msg.encode(sector_info);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}

			if (xQueueReceive( tanksStatusQueue, &watertank, 1 ) == pdPASS){

				outbound_msg.encode(watertank);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}

			if(xQueueReceive( pumpsStatusQueue, &encoded_pumps_status, 1 ) == pdPASS){

				outbound_msg.encodeGeneric(target_t::Pump, 255, encoded_pumps_status);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}

			while(xQueueReceive( plantsHealthQueue, &plant_info, 1 )){

				outbound_msg.encode(plant_info);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}

			if (xQueueReceive( batteryStatusQueue, &battery_info, 1 ) == pdPASS){

				outbound_msg.encode(battery_info);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}

			while(xQueueReceive( confirmationsQueue, &confirmation, 5 )){

				outbound_msg.encode(confirmation);
				radio1.TransmitPayload(outbound_msg.uplinkframe->buffer);

				/* Wait for data to be sent */
				do {
					/* Wait till sending */
					transmissionStatus = radio1.GetTransmissionStatus();
				} while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
			}
    	}

    	LEDToggle(8);
    	vTaskDelayUntil(&xLastWakeTime,xFrequencySeconds);
    }


}



