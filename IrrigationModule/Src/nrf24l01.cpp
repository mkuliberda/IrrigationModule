#include <nrf24l01.h>



#ifndef RPI

void NRF24L01::Init(SPI_HandleTypeDef *_spi, const struct gpio_s & _ce, const struct gpio_s & _csn){
	/* Init pins */
	this->ce.port = _ce.port;
	this->ce.pin = _ce.pin;
	this->pspi = _spi;
	this->csn.port = _csn.port;
	this->csn.pin = _csn.pin;
	
	/* CSN high = disable SPI */
	HAL_GPIO_WritePin(this->csn.port, this->csn.pin, GPIO_PIN_SET);
	
	/* CE low = disable TX/RX */
	HAL_GPIO_WritePin(this->ce.port, this->ce.pin, GPIO_PIN_RESET);
}

#else

void NRF24L01::Init(const int & _spifd, const int & _csn, const int & _ce){
	this->fd = _spifd;
	this->csn = _csn;
	this->ce = _ce;

}
#endif


bool& NRF24L01::Config(const uint8_t & _payloadsize, const uint8_t & _channel, const NRF24L01_OutputPower_t & _outpwr, const NRF24L01_DataRate_t & _datarate) {
	
	this->valid = true;

	/* Max payload is 32bytes */
	if (_payloadsize <= 32) {
		this->config_struct.PayloadSize = _payloadsize;
	}
	else this->config_struct.PayloadSize = 32;

	/* Reset nRF24L01+ to power on registers values */
	this->SoftwareReset();
	
	/* Channel select */
	if (this->SetChannel(_channel) != true){ return this->valid = false;}
	
	/* Set pipeline to max possible 32 bytes */
	this->WriteRegister(NRF24L01_REG_RX_PW_P0, this->config_struct.PayloadSize); // Auto-ACK pipe
	this->WriteRegister(NRF24L01_REG_RX_PW_P1, this->config_struct.PayloadSize); // Data payload pipe
	this->WriteRegister(NRF24L01_REG_RX_PW_P2, this->config_struct.PayloadSize);
	this->WriteRegister(NRF24L01_REG_RX_PW_P3, this->config_struct.PayloadSize);
	this->WriteRegister(NRF24L01_REG_RX_PW_P4, this->config_struct.PayloadSize);
	this->WriteRegister(NRF24L01_REG_RX_PW_P5, this->config_struct.PayloadSize);
	
	/* Set RF settings (datarate, output power) */
	if (this->SetRF(_datarate, _outpwr) != true) {return this->valid = false;}
	
	/* Config register */
	this->WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG);
	
	/* Enable auto-acknowledgment for all pipes */
	this->WriteRegister(NRF24L01_REG_EN_AA, 0x3F);
	if (this->ReadRegister(NRF24L01_REG_EN_AA) != 0x3F) {return this->valid = false;}
	
	/* Enable RX addresses */
	this->WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);
	if (this->ReadRegister(NRF24L01_REG_EN_RXADDR) != 0x3F) {return this->valid = false;}


	/* Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials */
	this->WriteRegister(NRF24L01_REG_SETUP_RETR, 0x4F);
	if (this->ReadRegister(NRF24L01_REG_SETUP_RETR) != 0x4F) {return this->valid = false;}

	
	/* Dynamic length configurations: No dynamic length */
	this->WriteRegister(NRF24L01_REG_DYNPD, (0 << NRF24L01_DPL_P0) | (0 << NRF24L01_DPL_P1) | (0 << NRF24L01_DPL_P2) | (0 << NRF24L01_DPL_P3) | (0 << NRF24L01_DPL_P4) | (0 << NRF24L01_DPL_P5));
	
	/* Clear FIFOs */
	this->FlushTX();
	this->FlushRX();
	
	/* Clear interrupts */
	this->ClearInterrupts();
	
	/* Go to RX mode */
	this->PowerUpRx();
	
	/* Return OK */
	return this->valid;
}

bool NRF24L01::SetMyAddress(uint8_t *adr) {
	this->CE_LOW();
	this->WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, adr, 5);
	this->CE_HIGH();
	//TODO: read register and check if values has been written, return true or false

	return true;
}

bool NRF24L01::SetTxAddress(uint8_t *adr) {
	this->WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	this->WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
	//TODO: read register and check if valus has been written, return true or false

	return true;
}

/* TODO: Flush FIFOs */
void NRF24L01::FlushTX(void){
	this->CSN_LOW();
#ifndef RPI
	uint8_t tx = NRF24L01_FLUSH_TX_MASK;
	HAL_SPI_Transmit(this->pspi, &tx, 1, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
}
void NRF24L01::FlushRX(void){
	this->CSN_LOW();
#ifndef RPI
	uint8_t tx = NRF24L01_FLUSH_RX_MASK;
	HAL_SPI_Transmit(this->pspi, &tx, 1, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
}

void NRF24L01::CSN_LOW(void){
#ifndef RPI
	HAL_GPIO_WritePin(this->csn.port, this->csn.pin, GPIO_PIN_RESET);
#else
	//TODO:
#endif
}

void NRF24L01::CSN_HIGH(void){
#ifndef RPI
	HAL_GPIO_WritePin(this->csn.port, this->csn.pin, GPIO_PIN_SET);
#else
	//TODO:
#endif
}

void NRF24L01::CE_LOW(void){
#ifndef RPI
	HAL_GPIO_WritePin(this->ce.port, this->ce.pin, GPIO_PIN_RESET);
#else
	//TODO:
#endif
}

void NRF24L01::CE_HIGH(void){
#ifndef RPI
	HAL_GPIO_WritePin(this->ce.port, this->ce.pin, GPIO_PIN_SET);
#else
	//TODO:
#endif
}

void NRF24L01::WriteBit(const uint8_t & _reg, const uint8_t & _bit, const uint8_t & _value) {
	uint8_t tmp;
	/* Read register */
	tmp = this->ReadRegister(_reg);
	/* Make operation */
	if (_value) {
		tmp |= 1 << _bit;
	} else {
		tmp &= ~(1 << _bit);
	}
	/* Write back */
	this->WriteRegister(_reg, tmp);
}

uint8_t NRF24L01::ReadBit(const uint8_t & _reg, const uint8_t & _bit) {
	uint8_t tmp;
	tmp = this->ReadRegister(_reg);
	if (!CHECK_BIT(tmp, _bit)) {
		return 0;
	}
	return 1;
}

uint8_t NRF24L01::ReadRegister(const uint8_t & _reg) {

	uint8_t value = 0;
	uint8_t tx = NRF24L01_NOP_MASK;
	uint8_t mask = NRF24L01_READ_REGISTER_MASK(_reg);

	this->CSN_LOW();
#ifndef RPI
	HAL_SPI_Transmit(this->pspi, &mask, 1, HAL_MAX_DELAY);
	HAL_SPI_TransmitReceive(this->pspi, &tx, &value, 1, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
	
	return value;
}

void NRF24L01::ReadRegisterMulti(const uint8_t & _reg, uint8_t* data, const uint8_t & _count) {

	uint8_t mask = NRF24L01_READ_REGISTER_MASK(_reg);

	this->CSN_LOW();
#ifndef RPI
	HAL_SPI_Transmit(this->pspi, &mask, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(this->pspi, data, _count, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
}

void NRF24L01::WriteRegister(const uint8_t & _reg, const uint8_t & _value) {

	uint8_t tx = NRF24L01_WRITE_REGISTER_MASK(_reg);
	uint8_t val = _value;

	this->CSN_LOW();
#ifndef RPI
	HAL_SPI_Transmit(this->pspi, &tx, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(this->pspi, &val, 1, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
}

void NRF24L01::WriteRegisterMulti(const uint8_t & _reg, uint8_t *data, const uint8_t & _count) {

	uint8_t mask = NRF24L01_WRITE_REGISTER_MASK(_reg);

	this->CSN_LOW();
#ifndef RPI
	HAL_SPI_Transmit(this->pspi, &mask, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(this->pspi, data, _count, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	this->CSN_HIGH();
}

void NRF24L01::PowerUpTx(void) {
	this->ClearInterrupts();
	this->WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

void NRF24L01::PowerUpRx(void) {
	/* Disable RX/TX mode */
	this->CE_LOW();
	/* Clear RX buffer */
	this->FlushRX();
	/* Clear interrupts */
	this->ClearInterrupts();
	/* Setup RX mode */
	this->WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | 1 << NRF24L01_PWR_UP | 1 << NRF24L01_PRIM_RX);
	/* Start listening */
	this->CE_HIGH();
}

void NRF24L01::PowerDown(void) {
	this->CE_LOW();
	this->WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, 0);
}

void NRF24L01::TransmitPayload(uint8_t *data) {

	uint8_t tx = NRF24L01_W_TX_PAYLOAD_MASK;

	/* Chip enable put to low, disable it */
	this->CE_LOW();
	/* Go to power up tx mode */
	this->PowerUpTx();
	/* Clear TX FIFO from NRF24L01+ */
	this->FlushTX();
	/* Send payload to nRF24L01+ */
	this->CSN_LOW();
#ifndef RPI
	/* Send write payload command */
	HAL_SPI_Transmit(this->pspi, &tx, 1, HAL_MAX_DELAY);
	/* Fill payload with data*/
	HAL_SPI_Transmit(this->pspi, data, this->config_struct.PayloadSize, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	/* Disable SPI */
	this->CSN_LOW();
	/* Send data! */
	this->CE_HIGH();
}

void NRF24L01::GetPayload(uint8_t* data) {

	uint8_t tx = NRF24L01_R_RX_PAYLOAD_MASK;

	/* Pull down chip select */
	this->CSN_LOW();
#ifndef RPI
	/* Send read payload command*/
	HAL_SPI_Transmit(this->pspi, &tx, 1, HAL_MAX_DELAY);
	/* Read payload */
	HAL_SPI_Receive(this->pspi, data, this->config_struct.PayloadSize, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	/* Pull up chip select */
	this->CSN_HIGH();
	/* Reset status register, clear RX_DR interrupt flag */
	this->WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));

}

uint8_t NRF24L01::DataReady(void) {

	uint8_t status = this->GetStatus();
	
	if (CHECK_BIT(status, NRF24L01_RX_DR)) {
		return 1;
	}
	return !this->RxFifoEmpty();
}

uint8_t NRF24L01::RxFifoEmpty(void) {
	uint8_t reg = this->ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return CHECK_BIT(reg, NRF24L01_RX_EMPTY);
}

uint8_t NRF24L01::GetStatus(void) {

	uint8_t status;
	uint8_t tx = NRF24L01_NOP_MASK;
	
	this->CSN_LOW();
#ifndef RPI
	/* First received byte is always status register */
	HAL_SPI_TransmitReceive(this->pspi, &tx, &status, 1, HAL_MAX_DELAY);
#else
	//TODO:
#endif
	/* Pull up chip select */
	this->CSN_HIGH();
	
	return status;
}

NRF24L01_Transmit_Status_t NRF24L01::GetTransmissionStatus(void) {

	uint8_t status = this->GetStatus();

	if (CHECK_BIT(status, NRF24L01_TX_DS)) {
		/* Successfully sent */
		return NRF24L01_Transmit_Status_Ok;
	} else if (CHECK_BIT(status, NRF24L01_MAX_RT)) {
		/* Message lost */
		return NRF24L01_Transmit_Status_Lost;
	}
	
	/* Still sending */
	return NRF24L01_Transmit_Status_Sending;
}

void NRF24L01::SoftwareReset(void) {

	uint8_t data[5];
	
	this->WriteRegister(NRF24L01_REG_CONFIG, 	NRF24L01_REG_DEFAULT_VAL_CONFIG);
	this->WriteRegister(NRF24L01_REG_EN_AA,		NRF24L01_REG_DEFAULT_VAL_EN_AA);
	this->WriteRegister(NRF24L01_REG_EN_RXADDR, NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	this->WriteRegister(NRF24L01_REG_SETUP_AW, 	NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	this->WriteRegister(NRF24L01_REG_SETUP_RETR,NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	this->WriteRegister(NRF24L01_REG_RF_CH, 	NRF24L01_REG_DEFAULT_VAL_RF_CH);
	this->WriteRegister(NRF24L01_REG_RF_SETUP, 	NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	this->WriteRegister(NRF24L01_REG_STATUS, 	NRF24L01_REG_DEFAULT_VAL_STATUS);
	this->WriteRegister(NRF24L01_REG_OBSERVE_TX,NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	this->WriteRegister(NRF24L01_REG_RPD, 		NRF24L01_REG_DEFAULT_VAL_RPD);
	
	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	this->WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);
	
	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	this->WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);
	
	this->WriteRegister(NRF24L01_REG_RX_ADDR_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	this->WriteRegister(NRF24L01_REG_RX_ADDR_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	this->WriteRegister(NRF24L01_REG_RX_ADDR_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	this->WriteRegister(NRF24L01_REG_RX_ADDR_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);
	
	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	this->WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);
	
	this->WriteRegister(NRF24L01_REG_RX_PW_P0, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	this->WriteRegister(NRF24L01_REG_RX_PW_P1, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	this->WriteRegister(NRF24L01_REG_RX_PW_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	this->WriteRegister(NRF24L01_REG_RX_PW_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	this->WriteRegister(NRF24L01_REG_RX_PW_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	this->WriteRegister(NRF24L01_REG_RX_PW_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	this->WriteRegister(NRF24L01_REG_FIFO_STATUS,NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	this->WriteRegister(NRF24L01_REG_DYNPD, 	NRF24L01_REG_DEFAULT_VAL_DYNPD);
	this->WriteRegister(NRF24L01_REG_FEATURE, 	NRF24L01_REG_DEFAULT_VAL_FEATURE);
}

uint8_t NRF24L01::GetRetransmissionsCount(void) {
	/* Low 4 bits */
	return this->ReadRegister(NRF24L01_REG_OBSERVE_TX) & 0x0F;
}

bool NRF24L01::SetChannel(const uint8_t & _channel) {

	uint8_t chan = 0;

	if (_channel <= 125 && _channel >= 0) {
		/* Store new channel setting */
		this->config_struct.Channel = _channel;
		/* Write channel */
		this->WriteRegister(NRF24L01_REG_RF_CH, _channel);
		chan = this->ReadRegister(NRF24L01_REG_RF_CH);

		return chan == _channel ? true : false;

	}else{
		return false;
	}
}

bool NRF24L01::SetRF(const NRF24L01_DataRate_t & _datarate, const NRF24L01_OutputPower_t & _outpwr) {

	uint8_t tmp = 0;
	config_struct.DataRate = _datarate;
	config_struct.OutPwr = _outpwr;
	
	if (_datarate == NRF24L01_DataRate_2M) {
		tmp |= 1 << NRF24L01_RF_DR_HIGH;
	} else if (_datarate == NRF24L01_DataRate_250k) {
		tmp |= 1 << NRF24L01_RF_DR_LOW;
	}
	/* If 1Mbps, all bits set to 0 */
	
	if (_outpwr == NRF24L01_OutputPower_0dBm) {
		tmp |= 3 << NRF24L01_RF_PWR;
	} else if (_outpwr == NRF24L01_OutputPower_M6dBm) {
		tmp |= 2 << NRF24L01_RF_PWR;
	} else if (_outpwr == NRF24L01_OutputPower_M12dBm) {
		tmp |= 1 << NRF24L01_RF_PWR;
	}
	
	this->WriteRegister(NRF24L01_REG_RF_SETUP, tmp);
	//TODO: read register and check if values are correct, return true or false

	return true;
}

uint8_t NRF24L01::ReadInterrupts(NRF24L01_IRQ_t* irq) {
	irq->Status = this->GetStatus();
	return irq->Status;
}

void NRF24L01::ClearInterrupts(void) {
	this->WriteRegister(0x07, 0x70);
}

