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

bool BinaryPump::init(const uint8_t & _id, const uint32_t & _idletimeRequiredSeconds, const uint32_t & _runtimeLimitSeconds, const struct gpio_s & _pinout, const struct gpio_s & _led){
	this->status.id = _id;
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
		if(this->runtimeGetSeconds() > this->runtimeLimitSeconds && this->status.forced == false) this->stop();
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
	this->status.forced = true;
}
void BinaryPump::forcestop(void){

	if (this->isRunning() == true) this->idletimeReset();

	HAL_GPIO_WritePin(this->pinout.port,this->pinout.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->led.port, this->led.pin, GPIO_PIN_SET);
	this->stateSet(pumpState_t::stopped);
	this->status.forced = true;
}


pumpState_t BinaryPump::stateGet(void){
	return this->state;
}

void BinaryPump::stateSet(const pumpState_t & _state){
	this->state = _state;
	this->status.state = static_cast<uint32_t>(_state);
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

sensorinterfacetype_t WaterLevelSensor::interfacetypeGet(void){
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

/******************************************/
/*! TemperatureSensor class implementation */
/******************************************/

temperaturesensortype_t TemperatureSensor::typeGet(void){
	return this->type;
}

sensorinterfacetype_t TemperatureSensor::interfacetypeGet(void){
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

bool DS18B20::isValid(void){
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

float WaterTank::temperatureCelsiusGet(void){
	return this->mean_watertemperatureCelsius;
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

bool WaterTank::waterlevelSensorAdd(const waterlevelsensortype_t & _sensortype){

	bool success = false;

	switch (_sensortype)
	{
	case waterlevelsensortype_t::WLS_optical:
		if (this->waterlevelSensorsCount < (this->waterlevelSensorsLimit+1))
		{
			OpticalWaterLevelSensor temp_sensor;
			this->vOpticalWLSensors.push_back(temp_sensor);
			this->waterlevelSensorsCount++;
			success = true;
		}
		break;

	case waterlevelsensortype_t::WLS_capacitive:
		break;

	case waterlevelsensortype_t::WLS_resistive:
		break;

	}

	return success;
}

bool WaterTank::temperatureSensorAdd(const temperaturesensortype_t & _sensortype){

	bool success = false;

	switch (_sensortype)
	{
	case temperaturesensortype_t::ds18b20:
		if (this->temperatureSensorsCount < (this->temperatureSensorsLimit+1))
		{
			DS18B20 temp_sensor;
			this->vTemperatureSensors.push_back(temp_sensor);
			this->temperatureSensorsCount++;
			success = true;
		}
		break;

	case temperaturesensortype_t::generic:
		break;
	}

	return success;
}

void pumpStateEncode(struct pumpstatus_s _pump, uint32_t & bitmask) {

	uint32_t tmp=0;

	switch (_pump.id)
	{
	case 0:
		tmp = bitmask | _pump.state;
		if (_pump.forced == true) 			tmp = bitmask | (1 << 6);
		if (_pump.cmd_consumed == true) 	tmp = bitmask | (1 << 7);
		break;
	case 1:
		tmp =  bitmask | _pump.state << 8;
		if (_pump.forced == true) 			tmp = bitmask | (1 << 14);
		if (_pump.cmd_consumed == true) 	tmp = bitmask | (1 << 15);
		break;
	case 2:
		tmp = bitmask | (_pump.state << 16);
		if (_pump.forced == true) 			tmp = bitmask | (1 << 22);
		if (_pump.cmd_consumed == true) 	tmp = bitmask | (1 << 23);
		break;
	case 3:
		tmp = bitmask | (_pump.state << 24);
		if (_pump.forced == true) 			tmp = bitmask | (1 << 30);
		if (_pump.cmd_consumed == true) 	tmp = bitmask | (1 << 31);
		break;
	default:
		break;
	}

	bitmask = tmp;

}
void pumpStateDecode(uint32_t bitmask){

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



