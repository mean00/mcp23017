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

static myMcp23017 *current=NULL;

static void _myInterrupt()
{
    if(!current) 
        return;
    current->interrupt();
}

/**
 * 
 * @param ctor
 * @param pinInterrupt : Pin that send input change (A0...A7)
 * @param Address : A2A1A0 , default is zero, real i2c address is 0x20+addr
 * @param w       : Wire interface, null uses Wire i.e. default i2c
 */
myMcp23017::myMcp23017(int pinInterrupt,uint8_t addr, WireBase *w)
{
    i2cAddress=(addr%8)+MCP23017_BASE_ADDRESS;
    wire=w;
    this->pinInterrupt=pinInterrupt;
    if(!wire)
        wire=&Wire;    
    init();
}

/**
 * \fn init
 * \brief set default sane value for most configuration
 */

void myMcp23017::init() 
{

	// set defaults:
	// "A" Pins are all input, "B" pins are all outputs
	writeRegister(MCP23017_IODIRA,0xff);
        // and all pullup
        writeRegister(MCP23017_GPPUA,0xff);
        writeRegister(MCP23017_OLATA,0xff);
        PortALatch=0xff;
        
        
        // B are all outputs
	writeRegister(MCP23017_IODIRB,0);
	// set to gnd by default
        writeRegister(MCP23017_OLATB,0);
        PortBValue=0;
        
        current=this;
}
/**
 * 
 * @param pin
 * @param onoff
 */
void      myMcp23017::digitalWrite(int pin, bool onoff)
{
    int msk=1<<pin;
    if(onoff) 
        PortBValue|=msk;
    else 
        PortBValue&=~msk;
    writeRegister(MCP23017_OLATB,PortBValue);     
}

/**
 * \fn start
 * \brief start the interrupt system
 */
void myMcp23017::start()
{
    // All interrupt on change for all A
    writeRegister(MCP23017_GPINTENA,0xff);
    // And use previous value
    writeRegister(MCP23017_INTCONA,0);
    
    noInterrupts();        
    pinMode(pinInterrupt,INPUT_PULLUP);
    attachInterrupt(pinInterrupt,_myInterrupt,CHANGE);    
    changed=false;
    interrupts();
}


/**
 * 
 */
void myMcp23017::interrupt()
{
    changed=true;
}
/**
 * 
 */
void myMcp23017::process()
{
    noInterrupts();       
    bool copy=changed;
    changed=false;
    interrupts();
    
    if(!copy) 
        return;
    
    int newValue=readRegister(MCP23017_OLATA);
    if(newValue!=PortALatch)
    {
        printf("Value change %x\n",newValue^PortALatch);
    }
    PortALatch=newValue;
}

/**
 * Reads a given register
 */
uint8_t myMcp23017::readRegister(uint8_t addr)
{
    // read the current GPINTEN
    wire->beginTransmission(i2cAddress);
    wire->write(addr);
    wire->endTransmission();
    wire->requestFrom(i2cAddress, 1);
    return wire->read();
}


/**
 * Writes a given register
 */
void myMcp23017::writeRegister(uint8_t regAddr, uint8_t regValue)
{
    // Write the register
    uint8_t txBuffer[2]={regAddr,regValue};
    wire->beginTransmission(i2cAddress);
    wire->write(txBuffer,2);
    wire->endTransmission();
}


// EOF


