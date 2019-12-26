/**
 * Irrigation control file with classes for sensors, tanks, pumps etc
 */

#include <irrigation.h>


/******************************/
/*! Pump class implementation */
/******************************/

bool Pump::init(void){
	return true;
}

bool Pump::isRunning(void){
	return stateGet() == pumpState_t::running ? true : false;
}

void Pump::run(const double & _dt){

}


/************************************/
/*! BinaryPump class implementation */
/************************************/

bool BinaryPump::init(const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led){
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = _idletimeRequiredSeconds;
	this->runtimeLimitSeconds = _runtimeLimitSeconds;
	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpState_t::stopped);

	return this->isRunning();
}

void BinaryPump::run(const double & _dt, const bool & _cmd_start, bool & cmd_consumed){

	switch (this->stateGet())
	{
	case pumpState_t::init:
		this->stop();
		cmd_consumed = true;
		break;

	case pumpState_t::waiting:
		this->idletimeIncrease(_dt);
		if(this->idletimeGetSeconds() > this->idletimeRequiredSeconds){
			if (this->start() == true ) cmd_consumed = true;
		}

		break;

	case pumpState_t::stopped:
		this->idletimeIncrease(_dt);
		if((_cmd_start == true) && (cmd_consumed == false) && (this->idletimeGetSeconds() > this->idletimeRequiredSeconds)){
			if (this->start() == true ) cmd_consumed = true;
		}
		else if((_cmd_start == true) && (cmd_consumed == false) && (this->idletimeGetSeconds() <= this->idletimeRequiredSeconds)){
			this->stateSet(pumpState_t::waiting);
		}
		break;

	case pumpState_t::running:
		this->runtimeIncrease(_dt);
		if(_cmd_start == true){
			cmd_consumed = true;
		}
		else{
			if(this->stop() == true) cmd_consumed = true;
		}
		if(this->runtimeGetSeconds() > this->runtimeLimitSeconds && this->forced == false) this->stop();
		break;

	default:
		break;
	}
}

bool BinaryPump::start(void){

	bool success = false;

	if(this->isRunning() == false){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
		this->stateSet(pumpState_t::running);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

bool BinaryPump::stop(void){

	bool success = false;

	if(this->isRunning() == true){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
		this->stateSet(pumpState_t::stopped);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

void BinaryPump::forcestart(void){

	if(this->isRunning() == false) this->runtimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
	this->stateSet(pumpState_t::running);
	this->forced = true;
}
void BinaryPump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpState_t::stopped);
	this->forced = true;
}


pumpState_t BinaryPump::stateGet(void){
	return this->state;
}

void BinaryPump::stateSet(const pumpState_t & _state){
	this->state = _state;
}

void BinaryPump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void BinaryPump::runtimeIncrease(const double & _dt){
	this->runtimeSeconds += _dt;
}

double BinaryPump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void BinaryPump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void BinaryPump::idletimeIncrease(const double & _dt){
	this->idletimeSeconds += _dt;
}

double BinaryPump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}




/******************************************/
/*! WaterLevelSensor class implementation */
/******************************************/

waterlevelsensorsubtype_t  WaterLevelSensor::subtypeGet(void){
	return this->subtype;
}

waterlevelsensortype_t WaterLevelSensor::typeGet(void){
	return this->type;
}

/*************************************************/
/*! OpticalWaterLevelSensor class implementation */
/*************************************************/

bool OpticalWaterLevelSensor::init(const float & _mountpositionMeters, const struct gpio_s & _pinout){
	this->mountpositionMeters = _mountpositionMeters;
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->read();
	return true;
}

const float OpticalWaterLevelSensor::mountpositionGet(void){
	return this->mountpositionMeters;
}

void OpticalWaterLevelSensor::read(void){
	if (HAL_GPIO_ReadPin(this->pinout.port, this->pinout.pin) == GPIO_PIN_SET) this->state = fixedwaterlevelsensorState_t::dry;
	else this->state = fixedwaterlevelsensorState_t::wet;
}

bool OpticalWaterLevelSensor::isValid(void){
	return this->state != fixedwaterlevelsensorState_t::undetermined ? true : false;
}

bool OpticalWaterLevelSensor::isSubmersed(void){
	this->read();
	return this->state == fixedwaterlevelsensorState_t::wet ? true : false;
}


/***********************************/
/*! WaterTank class implementation */
/***********************************/

bool WaterTank::init(void){

	return true;
}

double WaterTank::temperatureCelsiusGet(void){

	this->temperature = 12.7; //DS18B20 read function TBD
	return this->temperature;
}

void WaterTank::waterlevelSet(const contentlevel_t & _waterlevel){
	this->waterlevel= _waterlevel;
}

WaterTank::contentlevel_t WaterTank::waterlevelGet(void){
	return this->waterlevel;
}

void WaterTank::stateSet(const contentstate_t & _state){
	this->waterstate = _state;
}

WaterTank::contentstate_t WaterTank::stateGet(void){
	return this->waterstate;
}

bool WaterTank::checkStateOK(uint32_t & errcodeBitmask){

	/******************************errcodeBitmask****************************************
	 * *Upper 16 bits										Lower 16 bits
	 * 00000000 00000000 									00000000 00000000
	 * |||||||| ||||||||->water temperature too low	 (16)	|||||||| ||||||||->(0)
	 * |||||||| |||||||-->water temperature too high (17)	|||||||| |||||||-->(1)
	 * |||||||| ||||||--->water level too low		 (18)	|||||||| ||||||--->(2)
	 * |||||||| |||||---->							 (19)	|||||||| |||||---->(3)
	 * |||||||| ||||----->temperature sensor1 invalid(20)	|||||||| ||||----->(4)
	 * |||||||| |||------>temperature sensor2 invalid(21)	|||||||| |||------>(5)
	 * |||||||| ||------->wl sensor1 invalid         (22)	|||||||| ||------->(6)
	 * |||||||| |-------->wl sensor2 invalid         (23)	|||||||| |-------->(7)
	 * ||||||||---------->wl sensor3 invalid         (24)	||||||||---------->(8)
	 * |||||||----------->wl sensor4 invalid         (25)	|||||||----------->(9)
	 * ||||||------------>wl sensor5 invalid         (26)	||||||------------>(10)
	 * |||||------------->wl sensor6 invalid         (27)	|||||------------->(11)
	 * ||||-------------->wl sensor7 invalid         (28)	||||-------------->(12)
	 * |||--------------->wl sensor8 invalid         (29)	|||--------------->(13)
	 * ||---------------->wl sensor9 invalid         (30)	||---------------->(14)
	 * |----------------->wl sensor10 invalid        (31)	|----------------->(15)
	 */

	uint8_t temp_waterlevelPercent = 0;
	uint8_t waterlevelPercent = 0;
	bool 	isOK = true;
	errcodeBitmask = 0xFFFFFFFF;

	//TODO: fill errcode sensors
	if (this->temperatureSensorsCount > 0){

		double temperature = this->temperatureCelsiusGet();

		if(temperature < 0.0){
			this->stateSet(contentstate_t::frozen);
			errcodeBitmask = errcodeBitmask & (~(1 << (17))); //clear temp too high bit
			isOK = false;
		}
		else if (temperature > 100.0)
		{
			this->stateSet(contentstate_t::boiling);
			errcodeBitmask = errcodeBitmask & (~(1 << (16))); //clear temp too low bit
			isOK = false;
		}
		else{
			this->stateSet(contentstate_t::liquid);
			errcodeBitmask = errcodeBitmask & (~(1 << (16))); //clear temp too low bit
			errcodeBitmask = errcodeBitmask & (~(1 << (17))); //clear temp too high bit
		}
	}


	if (this->waterlevelSensorsCount > 0){
		uint8_t owls_count = this->vOpticalWLSensors.size();
		for(int i=0; i<owls_count; i++){
			if(this->vOpticalWLSensors[i].isValid() && this->vOpticalWLSensors[i].isSubmersed()){

				temp_waterlevelPercent = this->waterlevelConvertToPercent(this->vOpticalWLSensors[i].mountpositionGet());
				if(temp_waterlevelPercent > waterlevelPercent) waterlevelPercent = temp_waterlevelPercent;
			}
		}
	}

	if		(waterlevelPercent >= 98) 	{ this->waterlevelSet(contentlevel_t::full); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if	(waterlevelPercent > 90) 	{ this->waterlevelSet(contentlevel_t::above90); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 80) 	{ this->waterlevelSet(contentlevel_t::above80); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 70) 	{ this->waterlevelSet(contentlevel_t::above70); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 60) 	{ this->waterlevelSet(contentlevel_t::above60); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 50) 	{ this->waterlevelSet(contentlevel_t::above50); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 40) 	{ this->waterlevelSet(contentlevel_t::above40); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 30) 	{ this->waterlevelSet(contentlevel_t::above30); errcodeBitmask = errcodeBitmask & (~(1 << (18))); }
	else if (waterlevelPercent > 20) 	this->waterlevelSet(contentlevel_t::above20);
	else if (waterlevelPercent > 10) 	this->waterlevelSet(contentlevel_t::above10);
	else if (waterlevelPercent >= 0) 	{ this->waterlevelSet(contentlevel_t::empty); isOK = false; }

	return isOK;
}

uint8_t WaterTank::waterlevelConvertToPercent(const float & _valMeters){
	return static_cast<uint8_t>(_valMeters/this->tankheightMeters*100);
}

uint8_t WaterTank::waterlevelPercentGet(void){
	return static_cast<uint8_t>(this->waterlevel);
}


bool WaterTank::waterlevelSensorAdd(const waterlevelsensortype_t & _sensortype){

	bool success = false;

	switch (_sensortype)
	{
	case waterlevelsensortype_t::optical:
		if (this->waterlevelSensorsCount < (this->waterlevelSensorsLimit+1))
		{
			OpticalWaterLevelSensor temp_sensor;
			this->vOpticalWLSensors.push_back(temp_sensor);
			this->waterlevelSensorsCount++;
			success = true;
		}
		break;

	case waterlevelsensortype_t::capacitive:
		break;

	case waterlevelsensortype_t::resistive:
		break;

	}

	return success;
}

bool WaterTank::temperatureSensorAdd(const temperaturesensortype_t & _sensortype){
	return true;
}





///*! MoistureSensor template class implementation (Test only) */
//
//template <typename T> void MoistureSensor::rawreadingSet(T & _moisture_reading_raw){
//	this->moisture_reading_raw = _moisture_reading_raw;
//}
//
//
//	//using index_t = array<int, 10>::size_type;
//for (index_t i{ 0 }; i < _fixedwlsposition.size(); ++i)
//{
//	if ( _fixedwlsposition[i] > 0.0f)
//	{
		//OpticalWaterLevelSensor temp_WLSensor(_fixedwlsposition[i]);
		//this->vWLSensors.push_back(temp_WLSensor);
//	}
//}
//
//
///*! AnalogMoistureSensor class implementation (Test only TBD) */
//
//uint8_t AnalogMoistureSensor::moistureCalculatePercent(void){
//	this->moisture_percent = this->moisture_reading_raw;
//	return this->moisture_percent;
//}



