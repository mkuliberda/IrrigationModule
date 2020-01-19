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
	return stateGet() == pumpstate_t::running ? true : false;
}

void Pump::run(const double & _dt){

}

struct pumpstatus_s& Pump::statusGet(void){
	return this->status;
}



/************************************/
/*! BinaryPump class implementation */
/************************************/

bool BinaryPump::init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led){
	this->status.id = _id;
	this->pinout.pin = _pinout.pin;
	this->pinout.port = _pinout.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = _idletimeRequiredSeconds;
	this->runtimeLimitSeconds = _runtimeLimitSeconds;
	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
	this->stateSet(pumpstate_t::stopped);

	return true;
}

void BinaryPump::run(const double & _dt, const bool & _cmd_start, bool & cmd_consumed){

	switch (this->stateGet())
	{
	case pumpstate_t::init:
		this->stop();
		cmd_consumed = true;
		break;

	case pumpstate_t::waiting:
		this->idletimeIncrease(_dt);
		if(this->idletimeGetSeconds() > this->idletimeRequiredSeconds){
			this->start();
			cmd_consumed = true;
		}

		break;

	case pumpstate_t::stopped:
		this->idletimeIncrease(_dt);
		if((_cmd_start == true) && (this->idletimeGetSeconds() > this->idletimeRequiredSeconds)){
			this->start();
			cmd_consumed = true;
		}
		else if((_cmd_start == true) && (this->idletimeGetSeconds() <= this->idletimeRequiredSeconds)){
			this->stateSet(pumpstate_t::waiting);
		}
		break;

	case pumpstate_t::running:
		this->runtimeIncrease(_dt);
		if(_cmd_start == true){
			cmd_consumed = true;
		}
		else{
			this->stop();
			cmd_consumed = true;
		}
		if(this->runtimeGetSeconds() > this->runtimeLimitSeconds && this->status.forced == false) this->stop();
		break;

	default:
		break;
	}
}

bool BinaryPump::start(void){

	bool success = false;

	if(this->isRunning() == false){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
		this->stateSet(pumpstate_t::running);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

bool BinaryPump::stop(void){

	bool success = false;

	if(this->isRunning() == true){

		HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
		this->stateSet(pumpstate_t::stopped);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

void BinaryPump::forcestart(void){

	if(this->isRunning() == false) this->runtimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpstate_t::running);
	this->status.forced = true;
}
void BinaryPump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_RESET);
	this->stateSet(pumpstate_t::stopped);
	this->status.forced = true;
}


pumpstate_t& BinaryPump::stateGet(void){
	return this->state;
}

void BinaryPump::stateSet(const pumpstate_t & _state){
	this->state = _state;
	this->status.state = static_cast<uint32_t>(_state);
}

void BinaryPump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void BinaryPump::runtimeIncrease(const double & _dt){
	this->runtimeSeconds += _dt;
}

double& BinaryPump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void BinaryPump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void BinaryPump::idletimeIncrease(const double & _dt){
	this->idletimeSeconds += _dt;
}

double& BinaryPump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}

/************************************/
/*! DRV8833Pump class implementation */
/************************************/

bool DRV8833Pump::init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, \
		const array<struct gpio_s, 4> & _pinout, const struct gpio_s & _led, \
		const struct gpio_s & _fault, const struct gpio_s & _mode){
	this->status.id = _id;
	this->aIN[0].pin = _pinout[0].pin;
	this->aIN[0].port = _pinout[0].port;
	this->aIN[1].pin = _pinout[1].pin;
	this->aIN[1].port = _pinout[1].port;
	this->aIN[2].pin = _pinout[2].pin;
	this->aIN[2].port = _pinout[2].port;
	this->aIN[3].pin = _pinout[3].pin;
	this->aIN[3].port = _pinout[3].port;
	this->fault.pin = _fault.pin;
	this->fault.port = _fault.port;
	this->mode.pin = _mode.pin;
	this->mode.port = _mode.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = _idletimeRequiredSeconds;
	this->runtimeLimitSeconds = _runtimeLimitSeconds;
	//TODO: to implement, set stop here based on type dc/bldc
	this->stateSet(pumpstate_t::stopped);

	return true;
}

bool DRV8833Pump::init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, \
		const array<struct gpio_s, 2> & _pinout, const struct gpio_s & _led, \
		const struct gpio_s & _fault, const struct gpio_s & _mode){
	this->status.id = _id;
	this->aIN[0].pin = _pinout[0].pin;
	this->aIN[0].port = _pinout[0].port;
	this->aIN[1].pin = _pinout[1].pin;
	this->aIN[1].port = _pinout[1].port;
	this->aIN[2].pin = 0;
	this->aIN[2].port = nullptr;
	this->aIN[3].pin = 0;
	this->aIN[3].port = nullptr;
	this->fault.pin = _fault.pin;
	this->fault.port = _fault.port;
	this->mode.pin = _mode.pin;
	this->mode.port = _mode.port;
	this->led.pin = _led.pin;
	this->led.port = _led.port;
	this->idletimeRequiredSeconds = _idletimeRequiredSeconds;
	this->runtimeLimitSeconds = _runtimeLimitSeconds;
	//TODO: to implement, set stop here based on type dc/bldc
	this->stateSet(pumpstate_t::stopped);

	return true;
}

void DRV8833Pump::run(const double & _dt, const pumpcmd_t & _cmd, bool & cmd_consumed){
	//TODO: implement this
}

bool DRV8833Pump::start(void){

	bool success = false;

	if(this->isRunning() == false){

		//TODO: to implement based on type, dc/bldc
		this->stateSet(pumpstate_t::running);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

bool DRV8833Pump::stop(void){

	bool success = false;

	if(this->isRunning() == true){

		//TODO: to implement based on type, dc/bldc
		this->stateSet(pumpstate_t::stopped);
		this->idletimeReset();
		this->runtimeReset();
		success = true;
	}

	return success;
}

bool DRV8833Pump::reverse(void){
	//TODO: to implement based on type, dc/bldc
	return true;
}

void DRV8833Pump::forcestart(void){

	if(this->isRunning() == false) this->runtimeReset();

	//TODO: to implement based on type, dc/bldc
	this->stateSet(pumpstate_t::running);
	this->status.forced = true;
}
void DRV8833Pump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	//TODO: to implement based on type, dc/bldc
	this->stateSet(pumpstate_t::stopped);
	this->status.forced = true;
}

void DRV8833Pump::forcereverse(void){
	//TODO: to implement based on type, dc/bldc
}

void DRV8833Pump::sleepmodeSet(void){
	//TODO: to implement
}
bool DRV8833Pump::faultCheck(void){
	//TODO: to implement
	return 0;
}


pumpstate_t& DRV8833Pump::stateGet(void){
	return this->state;
}

void DRV8833Pump::stateSet(const pumpstate_t & _state){
	this->state = _state;
	this->status.state = static_cast<uint32_t>(_state);
}

void DRV8833Pump::runtimeReset(void){
	this->runtimeSeconds = 0.0;
}

void DRV8833Pump::runtimeIncrease(const double & _dt){
	this->runtimeSeconds += _dt;
}

double& DRV8833Pump::runtimeGetSeconds(void){
	return this->runtimeSeconds;
}

void DRV8833Pump::idletimeReset(void){
	this->idletimeSeconds = 0.0;
}

void DRV8833Pump::idletimeIncrease(const double & _dt){
	this->idletimeSeconds += _dt;
}

double& DRV8833Pump::idletimeGetSeconds(void){
	return this->idletimeSeconds;
}

void DRV8833Pump::revtimeReset(void){
	this->revtimeSeconds = 0.0;
}

void DRV8833Pump::revtimeIncrease(const double & _dt){
	this->revtimeSeconds += _dt;
}

double& DRV8833Pump::revtimeGetSeconds(void){
	return this->revtimeSeconds;
}

/***********************************/
/*! MoistureSensor class implementation */
/***********************************/

moisturesensortype_t& MoistureSensor::typeGet(void){
	return this->type;
}

sensorinterfacetype_t& MoistureSensor::interfacetypeGet(void){
	return this->interfacetype;
}

float& MoistureSensor::percentGet(void){
	return this->moisturePercent;
}

/***********************************/
/*! AnalogDMAMoistureSensor class implementation */
/***********************************/

void AnalogDMAMoistureSensor::voltsUpdate(void){
	this->moistureVolts = this->moistureRaw * 3.0f/4095.0f;
}

void AnalogDMAMoistureSensor::percentUpdate(void){
	this->moisturePercent = this->moistureRaw * 100.0f/4095.0f;
}

float AnalogDMAMoistureSensor::read(void){
	return 0; //Not used in this type of sensor
}

bool& AnalogDMAMoistureSensor::isValid(void){
	return this->valid;
}

void AnalogDMAMoistureSensor::rawUpdate(const uint16_t & _raw_value){
	this->moistureRaw = _raw_value;
	this->percentUpdate();
	this->voltsUpdate();
}

float& AnalogDMAMoistureSensor::voltsGet(void){
	return this->moistureVolts;
}

/******************************************/
/*! WaterLevelSensor class implementation */
/******************************************/

waterlevelsensorsubtype_t&  WaterLevelSensor::subtypeGet(void){
	return this->subtype;
}

waterlevelsensortype_t& WaterLevelSensor::typeGet(void){
	return this->type;
}

sensorinterfacetype_t& WaterLevelSensor::interfacetypeGet(void){
	return this->interfacetype;
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

const float& OpticalWaterLevelSensor::mountpositionGet(void){
	return this->mountpositionMeters;
}

void OpticalWaterLevelSensor::read(void){
	if (HAL_GPIO_ReadPin(this->pinout.port, this->pinout.pin) == GPIO_PIN_SET) this->state = fixedwaterlevelsensorstate_t::dry;
	else this->state = fixedwaterlevelsensorstate_t::wet;
}

bool OpticalWaterLevelSensor::isValid(void){
	return this->state != fixedwaterlevelsensorstate_t::undetermined ? true : false;
}

bool OpticalWaterLevelSensor::isSubmersed(void){
	this->read();
	return this->state == fixedwaterlevelsensorstate_t::wet ? true : false;
}

/******************************************/
/*! TemperatureSensor class implementation */
/******************************************/

temperaturesensortype_t& TemperatureSensor::typeGet(void){
	return this->type;
}

sensorinterfacetype_t& TemperatureSensor::interfacetypeGet(void){
	return this->interfacetype;
}

/******************************************/
/*! DS18B20 class implementation */
/******************************************/
bool DS18B20::init(const struct gpio_s & _gpio, TIM_HandleTypeDef* _tim_baseHandle){

	this->gpio.port = _gpio.port;
	this->gpio.pin = _gpio.pin;
	this->timer = _tim_baseHandle;
	this->valid = this->prep();
	return this->valid;
}

bool DS18B20::prep(void){

	this->gpioSetOutput ();   // set the pin as output
	HAL_GPIO_WritePin (this->gpio.port, this->gpio.pin, GPIO_PIN_RESET);  // pull the pin low
	this->delay_us (480);   // delay according to datasheet

	this->gpioSetInput ();    // set the pin as input
	this->delay_us (80);    // delay according to datasheet

	if (!(HAL_GPIO_ReadPin (this->gpio.port, this->gpio.pin)))    // if the pin is low i.e the presence pulse is there
	{
		this->delay_us (400);  // wait for 400 us
		return true;
	}

	else
	{
		this->delay_us (400);
		return false;
	}
}

/* delay in microseconds */
void DS18B20::delay_us (const uint32_t & _us){
	__HAL_TIM_SET_COUNTER(this->timer,0);
	while ((__HAL_TIM_GET_COUNTER(this->timer))<_us);
}

void DS18B20::gpioSetInput (void){

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = this->gpio.pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(this->gpio.port, &GPIO_InitStruct);
}


void DS18B20::gpioSetOutput (void){

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = this->gpio.pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(this->gpio.port, &GPIO_InitStruct);
}

void DS18B20::write (const uint8_t & _data){

	this->gpioSetOutput();   // set as output

	for (int i=0; i<8; i++)
	{

		if ((_data & (1<<i))!=0)  // if the bit is high
		{
			// write 1

			this->gpioSetOutput();  // set as output
			HAL_GPIO_WritePin (this->gpio.port, this->gpio.pin, GPIO_PIN_RESET);  // pull the pin LOW
			this->delay_us(1);  // wait for  us

			this->gpioSetInput();  // set as input
			this->delay_us(60);  // wait for 60 us
		}

		else  // if the bit is low
		{
			// write 0

			this->gpioSetOutput();
			HAL_GPIO_WritePin (this->gpio.port, this->gpio.pin, GPIO_PIN_RESET);  // pull the pin LOW
			this->delay_us(60);  // wait for 60 us

			this->gpioSetInput();
		}
	}
}


uint8_t DS18B20::read (void){

	uint8_t value=0;

	this->gpioSetInput();

	for (int i=0;i<8;i++)
	{
		this->gpioSetOutput();   // set as output

		HAL_GPIO_WritePin (this->gpio.port, this->gpio.pin, GPIO_PIN_RESET);  // pull the data pin LOW
		this->delay_us(2);  // wait for 2 us

		this->gpioSetInput();  // set as input
		if (HAL_GPIO_ReadPin (this->gpio.port, this->gpio.pin))  // if the pin is HIGH
		{
			value |= 1<<i;  // read = 1
		}
		this->delay_us(60);  // wait for 60 us
	}
	return value;
}

bool& DS18B20::isValid(void){
	return this->valid;
}

float DS18B20::temperatureCelsiusRead(void){

	uint8_t temp_l = 0, temp_h = 0;
	uint16_t temp = 0;

	this->prep (); //TODO: is this required here?
	this->write (0xCC);  // skip ROM
	this->write (0x44);  // convert t

	HAL_Delay (800);

	this->prep (); //TODO: is this required here?
	this->write (0xCC);  // skip ROM
	this->write (0xBE);  // Read Scratchpad

	temp_l = this->read();
	temp_h = this->read();
	temp = (temp_h<<8)|temp_l;

	return static_cast<float> (temp/16);
}

/***********************************/
/*! WaterTank class implementation */
/***********************************/

bool WaterTank::init(void){
	return true;
}

float& WaterTank::temperatureCelsiusGet(void){
	return this->mean_watertemperatureCelsius;
}

void WaterTank::waterlevelSet(const contentlevel_t & _waterlevel){
	this->waterlevel= _waterlevel;
}

WaterTank::contentlevel_t& WaterTank::waterlevelGet(void){
	return this->waterlevel;
}

void WaterTank::stateSet(const contentstate_t & _state){
	this->waterstate = _state;
}

WaterTank::contentstate_t& WaterTank::stateGet(void){
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
	bitset<32> errcode; errcode.set();  //initialize bitset and set all bits to 1


	if (this->temperatureSensorsCount > 0){

		const uint8_t temps_count = this->vTemperatureSensors.size();
		vector <float> vTemperature;

		for(uint8_t i=0; i<temps_count; i++){
			if(this->vTemperatureSensors[i].isValid() == true){
				vTemperature[i] = this->vTemperatureSensors[i].temperatureCelsiusRead();
				errcode.reset(20+i);
			}
		}

		this->mean_watertemperatureCelsius = (accumulate(vTemperature.begin(), vTemperature.end(), 0))/vTemperature.size();

		if(this->mean_watertemperatureCelsius < 0.0){
			this->stateSet(contentstate_t::frozen);
			errcode.reset(17);
			isOK = false;
		}
		else if (this->mean_watertemperatureCelsius > 100.0)
		{
			this->stateSet(contentstate_t::boiling);
			errcode.reset(16);
			isOK = false;
		}
		else{
			this->stateSet(contentstate_t::liquid);
			errcode.reset(16);
			errcode.reset(17);
		}
	}
	else{
		//let's let it work without temperature sensor for now, but with errorcodes
		this->stateSet(contentstate_t::liquid);
	}


	if (this->waterlevelSensorsCount > 0){
		uint8_t owls_count = this->vOpticalWLSensors.size();
		for(uint8_t i=0; i<owls_count; i++){
			if(this->vOpticalWLSensors[i].isValid() == true){
				errcode.reset(22+i);
				if (this->vOpticalWLSensors[i].isSubmersed()){
					temp_waterlevelPercent = this->waterlevelConvertToPercent(this->vOpticalWLSensors[i].mountpositionGet());
					if(temp_waterlevelPercent > waterlevelPercent) waterlevelPercent = temp_waterlevelPercent;
				}
			}
		}
	}

	if		(waterlevelPercent >= 98) 	{ this->waterlevelSet(contentlevel_t::full); errcode.reset(18); }
	else if	(waterlevelPercent > 90) 	{ this->waterlevelSet(contentlevel_t::above90); errcode.reset(18); }
	else if (waterlevelPercent > 80) 	{ this->waterlevelSet(contentlevel_t::above80); errcode.reset(18); }
	else if (waterlevelPercent > 70) 	{ this->waterlevelSet(contentlevel_t::above70); errcode.reset(18); }
	else if (waterlevelPercent > 60) 	{ this->waterlevelSet(contentlevel_t::above60); errcode.reset(18); }
	else if (waterlevelPercent > 50) 	{ this->waterlevelSet(contentlevel_t::above50); errcode.reset(18); }
	else if (waterlevelPercent > 40) 	{ this->waterlevelSet(contentlevel_t::above40); errcode.reset(18); }
	else if (waterlevelPercent > 30) 	{ this->waterlevelSet(contentlevel_t::above30); errcode.reset(18); }
	else if (waterlevelPercent > 20) 	this->waterlevelSet(contentlevel_t::above20);
	else if (waterlevelPercent > 10) 	this->waterlevelSet(contentlevel_t::above10);
	else if (waterlevelPercent >= 0) 	{ this->waterlevelSet(contentlevel_t::empty); isOK = false; }

	errcodeBitmask = errcode.to_ulong();

	return isOK;
}

uint8_t WaterTank::waterlevelConvertToPercent(const float & _valMeters){
	return static_cast<uint8_t>(_valMeters/this->tankheightMeters*100);
}

uint8_t WaterTank::waterlevelPercentGet(void){
	return static_cast<uint8_t>(this->waterlevel);
}

bool WaterTank::waterlevelSensorCreate(const waterlevelsensortype_t & _sensortype){

	bool success = true;

	switch (_sensortype)
	{
	case waterlevelsensortype_t::optical:
		if (this->waterlevelSensorsCount < (this->waterlevelSensorsLimit+1))
		{
			OpticalWaterLevelSensor temp_sensor;
			this->vOpticalWLSensors.push_back(temp_sensor);
			this->waterlevelSensorsCount++;
		}
		else
		{
			success = false;
		}
		break;

	case waterlevelsensortype_t::capacitive:
		success = false;
		break;

	case waterlevelsensortype_t::resistive:
		success = false;
		break;

	default:
		success = false;
		break;
	}

	return success;
}

bool WaterTank::temperatureSensorCreate(const temperaturesensortype_t & _sensortype){

	bool success = true;

	switch (_sensortype)
	{
	case temperaturesensortype_t::ds18b20:
		if (this->temperatureSensorsCount < (this->temperatureSensorsLimit+1))
		{
			DS18B20 temp_sensor;
			this->vTemperatureSensors.push_back(temp_sensor);
			this->temperatureSensorsCount++;
		}
		else
		{
			success = false;
		}
		break;

	case temperaturesensortype_t::generic:
		success = false;
		break;

	default:
		success = false;
		break;
	}

	return success;
}

/***********************************/
/*! PumpController class implementation */
/***********************************/

uint8_t PumpController::update(const double & _dt, const bool & _activate_watering){

	bool consumed = false;
	bitset<8> errcode;
	/*******errcode**********
	 * 00000000
	 * ||||||||->(0) 1 if cmd not consumed
	 * |||||||-->(1)
	 * ||||||--->(2)
	 * |||||---->(3)
	 * ||||----->(4)
	 * |||------>(5) 1 if none of avbl pumps was correctly initialized/created
	 * ||------->(6) 1 if controller is in wrong or not avbl mode
	 * |-------->(7) 1 if pumpsCount is 0
	 *************************/

	if(this->pumpsCount > 0)
	{
		switch (this->mode)
		{

		case pumpcontrollermode_t::init:
			errcode.set(7,true);
			break;

		case pumpcontrollermode_t::external:
				if (this->pBinPump != nullptr)
				{
					if(_activate_watering)
					{
						this->pBinPump->run(_dt, true, consumed);
						if(consumed == false) errcode.set(1,true);
					}
					else
					{
						this->pBinPump->run(_dt, false, consumed);
						if(consumed == false) errcode.set(1,true);
					}
				}
				else if (this->p8833Pump != nullptr)
				{
					if(_activate_watering)
					{
						this->p8833Pump->run(_dt, pumpcmd_t::start,consumed);
						if(consumed == false) errcode.set(1,true);
					}
					else
					{
						this->p8833Pump->run(_dt, pumpcmd_t::stop,consumed);
					}
				}
				else errcode.set(6,true);

			break;

		case pumpcontrollermode_t::manual:
			errcode.set(7,true);
			break;

		case pumpcontrollermode_t::automatic:
			errcode.set(7,true);
			break;

		case pumpcontrollermode_t::sleep:
			errcode.set(7,true);
			break;

		default:
			errcode.set(7,true);
			break;
		}
	}
	else errcode.set(8,true);

	return static_cast<uint8_t>(errcode.to_ulong());
}

bool PumpController::pumpCreate(const pumptype_t & _pumptype){

	bool success = true;

	switch(_pumptype){
	case pumptype_t::binary:
		if (this->pumpsCount < (this->pumpsLimit+1))
		{
			this->pBinPump = new BinaryPump();
			this->pumpsCount++;
		}
		else
		{
			success = false;
		}
		break;

	case pumptype_t::generic:
		success = false;
		break;

	case pumptype_t::drv8833_dc:
		if (this->pumpsCount < (this->pumpsLimit+1))
		{
			this->p8833Pump = new DRV8833Pump(motortype_t::dc_motor);
			this->pumpsCount++;
		}
		else
		{
			success = false;
		}
		break;

	case pumptype_t::drv8833_bldc:
		if (this->pumpsCount < (this->pumpsLimit+1))
		{
			this->p8833Pump = new DRV8833Pump(motortype_t::bldc_motor);
			this->pumpsCount++;
		}
		else
		{
			success = false;
		}
		break;

	default:
		success = false;
		break;
	}

	return success;
}

bool PumpController::moisturesensorCreate(const moisturesensortype_t & _sensortype){
	bool success = true;

	switch(_sensortype){
	case moisturesensortype_t::generic:
		success = false;
		break;

	case moisturesensortype_t::capacitive_noshield:
		if (this->moisturesensorsCount < (this->moisturesensorsLimit+1))
		{
			AnalogDMAMoistureSensor temp_sensor;
			this->vDMAMoistureSensor.push_back(temp_sensor);
			this->moisturesensorsCount++;
		}
		else
		{
			success = false;
		}
		break;

	default:
		success = false;
		break;
	}

	return success;
}

bool PumpController::modeSet(const pumpcontrollermode_t & _mode){

	bool changed = true;

	if (this->mode != _mode && _mode != pumpcontrollermode_t::init)
	{
		this->mode = _mode;
	}
	else changed = false;

	return changed;
}

const pumpcontrollermode_t&	PumpController::modeGet(void){
	return this->mode;
}


void pumpStateEncode(const struct pumpstatus_s & _pump, uint32_t & status) {

	switch (_pump.id)
	{
	case 0:
		status |= _pump.state;
		if (_pump.forced == true) 			status |= (1 << 6);
		if (_pump.cmd_consumed == true) 	status |= (1 << 7);
		break;
	case 1:
		status |= _pump.state << 8;
		if (_pump.forced == true) 			status |= (1 << 14);
		if (_pump.cmd_consumed == true) 	status |= (1 << 15);
		break;
	case 2:
		status |= (_pump.state << 16);
		if (_pump.forced == true) 			status |= (1 << 22);
		if (_pump.cmd_consumed == true) 	status |= (1 << 23);
		break;
	case 3:
		status |= (_pump.state << 24);
		if (_pump.forced == true) 			status |= (1 << 30);
		if (_pump.cmd_consumed == true) 	status |= (1 << 31);
		break;
	default:
		break;
	}

}

void pumpStateDecode(array<struct pumpstatus_s,4> & a_pump, const bitset<32> & _status) {

	const bitset<32> pumpstatemask(0x0000000F);
	bitset<32> tmp;

	for (uint8_t i = 0; i < 4; i++)
	{
		tmp = _status;
		if(i>0) tmp >>= 8*i;
		tmp &= pumpstatemask;
		a_pump[i].id = i;
		a_pump[i].state = tmp.to_ulong();
		if (_status.test(6)) a_pump[i].forced = true;
		if (_status.test(7)) a_pump[i].cmd_consumed = true;
	}
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



