/*************************************************** 
 This is a library for the MCP23017 i2c port expander

 These displays use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/


#include "Arduino.h"
#include "mcp23017.h"
#include "mcp23017_internal.h"

#define wiresend(x) wire->write((uint8_t) x)
#define wirerecv()  wire->read()


/**
 * Bit number associated to a give Pin
 */
uint8_t myMcp23017::bitForPin(uint8_t pin)
{
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t myMcp23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr)
{
	return(pin<8) ?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t myMcp23017::readRegister(uint8_t addr)
{
	// read the current GPINTEN
	wire->beginTransmission(i2cAddress);
	wiresend(addr);
	wire->endTransmission();
	wire->requestFrom(i2cAddress, 1);
	return wirerecv();
}


/**
 * Writes a given register
 */
void myMcp23017::writeRegister(uint8_t regAddr, uint8_t regValue)
{
	// Write the register
	wire->beginTransmission(i2cAddress);
	wiresend(regAddr);
	wiresend(regValue);
	wire->endTransmission();
}


/**
 * Helper to update a single bit of an A/B register.
 * - Reads the current register value
 * - Writes the new register value
 */
void myMcp23017::updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr) 
{
	uint8_t regValue;
	uint8_t regAddr=regForPin(pin,portAaddr,portBaddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	// set the value for the particular bit
	bitWrite(regValue,bit,pValue);

	writeRegister(regAddr,regValue);
}

////////////////////////////////////////////////////////////////////////////////


myMcp23017::myMcp23017(uint8_t addr, WireBase *w)
{
    i2cAddress=(addr%8)+MCP23017_BASE_ADDRESS;
    wire=w;
    if(!wire)
        wire=&Wire;
}

/**
 * Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
 */
void myMcp23017::begin() 
{

	wire->begin();
	// set defaults:
	// "A" Pins are all input, "B" pins are all outputs
	writeRegister(MCP23017_IODIRA,0xff);
	writeRegister(MCP23017_IODIRB,0);
        // and all pullup
        writeRegister(MCP23017_GPPUA,0xff);
	// set to gnd by default
        writeRegister(MCP23017_OLATB,0);
        

}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void myMcp23017::pinMode(uint8_t p, uint8_t d) 
{
	updateRegisterBit(p,(d==INPUT),MCP23017_IODIRA,MCP23017_IODIRB);
}

/**
 * Reads all 16 pins (port A and B) into a single 16 bits variable.
 */
uint16_t myMcp23017::readGPIOAB() 
{
	uint16_t ba = 0;
	uint8_t a;

	// read the current GPIO output latches
	wire->beginTransmission(i2cAddress);
	wiresend(MCP23017_GPIOA);
	wire->endTransmission();

	wire->requestFrom(i2cAddress, 2);
	a = wirerecv();
	ba = wirerecv();
	ba <<= 8;
	ba |= a;

	return ba;
}

/**
 * Read a single port, A or B, and return its current 8 bit value.
 * Parameter b should be 0 for GPIOA, and 1 for GPIOB.
 */
uint8_t myMcp23017::readGPIO(uint8_t b) 
{

	// read the current GPIO output latches
	wire->beginTransmission(i2cAddress);
	if (b == 0)
		wiresend(MCP23017_GPIOA);
	else {
		wiresend(MCP23017_GPIOB);
	}
	wire->endTransmission();

	wire->requestFrom(i2cAddress, 1);
	return wirerecv();
}

/**
 * Writes all the pins in one go. This method is very useful if you are implementing a multiplexed matrix and want to get a decent refresh rate.
 */
void myMcp23017::writeGPIOAB(uint16_t ba) 
{
	wire->beginTransmission(i2cAddress);
	wiresend(MCP23017_GPIOA);
	wiresend(ba & 0xFF);
	wiresend(ba >> 8);
	wire->endTransmission();
}

void myMcp23017::digitalWrite(uint8_t pin, uint8_t d) {
	uint8_t gpio;
	uint8_t bit=bitForPin(pin);


	// read the current GPIO output latches
	uint8_t regAddr=regForPin(pin,MCP23017_OLATA,MCP23017_OLATB);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio,bit,d);

	// write the new GPIO
	regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	writeRegister(regAddr,gpio);
}

void myMcp23017::pullUp(uint8_t p, uint8_t d) 
{
	updateRegisterBit(p,d,MCP23017_GPPUA,MCP23017_GPPUB);
}

uint8_t myMcp23017::digitalRead(uint8_t pin) 
{
	uint8_t bit=bitForPin(pin);
	uint8_t regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	return (readRegister(regAddr) >> bit) & 0x1;
}

/**
 * Configures the interrupt system. both port A and B are assigned the same configuration.
 * Mirroring will OR both INTA and INTB pins.
 * Opendrain will set the INT pin to value or open drain.
 * polarity will set LOW or HIGH on interrupt.
 * Default values after Power On Reset are: (false,flase, LOW)
 * If you are connecting the INTA/B pin to arduino 2/3, you should configure the interupt handling as FALLING with
 * the default configuration.
 */
void myMcp23017::setupInterrupts(uint8_t mirroring, uint8_t openDrain, uint8_t polarity)
{
	// configure the port A
	uint8_t ioconfValue=readRegister(MCP23017_IOCONA);
	bitWrite(ioconfValue,6,mirroring);
	bitWrite(ioconfValue,2,openDrain);
	bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONA,ioconfValue);

	// Configure the port B
	ioconfValue=readRegister(MCP23017_IOCONB);
	bitWrite(ioconfValue,6,mirroring);
	bitWrite(ioconfValue,2,openDrain);
	bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONB,ioconfValue);
}

/**
 * Set's up a pin for interrupt. uses arduino MODEs: CHANGE, FALLING, RISING.
 *
 * Note that the interrupt condition finishes when you read the information about the port / value
 * that caused the interrupt or you read the port itself. Check the datasheet can be confusing.
 *
 */
void myMcp23017::setupInterruptPin(uint8_t pin, uint8_t mode) 
{

	// set the pin interrupt control (0 means change, 1 means compare against given value);
	updateRegisterBit(pin,(mode!=CHANGE),MCP23017_INTCONA,MCP23017_INTCONB);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt

	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
	updateRegisterBit(pin,(mode==FALLING),MCP23017_DEFVALA,MCP23017_DEFVALB);

	// enable the pin for interrupt
	updateRegisterBit(pin,HIGH,MCP23017_GPINTENA,MCP23017_GPINTENB);

}

uint8_t myMcp23017::getLastInterruptPin()
{
	uint8_t intf;

	// try port A
	intf=readRegister(MCP23017_INTFA);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i;

	// try port B
	intf=readRegister(MCP23017_INTFB);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i+8;

	return MCP23017_INT_ERR;

}
uint8_t myMcp23017::getLastInterruptPinValue()
{
	uint8_t intPin=getLastInterruptPin();
	if(intPin!=MCP23017_INT_ERR){
		uint8_t intcapreg=regForPin(intPin,MCP23017_INTCAPA,MCP23017_INTCAPB);
		uint8_t bit=bitForPin(intPin);
		return (readRegister(intcapreg)>>bit) & (0x01);
	}

	return MCP23017_INT_ERR;
}


