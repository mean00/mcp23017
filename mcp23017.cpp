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
#include "mcp23017_impl.h"

static myMcp23017Impl *current=NULL;

/**
 * 
 * @param pinInterrupt
 * @param i2cAdr
 * @param wire
 * @return 
 */
myMcp23017 *myMcp23017::create(int pinInterrupt, int i2cAdr, WireBase *wire)
{
    return new myMcp23017Impl(pinInterrupt, i2cAdr,wire);
}

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
myMcp23017Impl::myMcp23017Impl(int pinInterrupt,uint8_t addr, WireBase *w)
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

void myMcp23017Impl::init() 
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
 * \fn start
 * \brief start the interrupt system
 */
void myMcp23017Impl::start()
{
    // All interrupt on change for all A
    writeRegister(MCP23017_GPINTENA,0xff);
    // And use previous value
    writeRegister(MCP23017_INTCONA,0);
    
    noInterrupts();        
    pinMode(pinInterrupt,INPUT_PULLUP);
    attachInterrupt(pinInterrupt,_myInterrupt,FALLING);    
    changed=false;
    interrupts();
    
}


/**
 * 
 * @param pin
 * @param onoff
 */
void      myMcp23017Impl::digitalWrite(int pin, bool onoff)
{
    int msk=1<<pin;
    if(onoff) 
        PortBValue|=msk;
    else 
        PortBValue&=~msk;
    writeRegister(MCP23017_OLATB,PortBValue);     
}
/**
 * 
 */
void myMcp23017Impl::interrupt()
{
    changed=true;
    detachInterrupt(pinInterrupt);    
}
/**
 * 
 */
void myMcp23017Impl::process()
{
    noInterrupts();       
    bool copy=changed;
    changed=false;
    interrupts();
    if(!copy) 
        return;
    // Ok an interrupt occured, process it and re-enable the interrupt
    
    // MCP23017_INTCAPA = value when interrupt occured
    // MCP23017_GPIOA = value now
    int newValue=readRegister(MCP23017_INTCAPA) ; //MCP23017_INTCAPA);
    if(newValue!=PortALatch)
    {
        printf("Value change %x\n",newValue^PortALatch);
    }
    PortALatch=newValue;
    attachInterrupt(pinInterrupt,_myInterrupt,FALLING);    
}

/**
 * Reads a given register
 */
uint8_t myMcp23017Impl::readRegister(int addr)
{
    
    wire->beginTransmission(i2cAddress);
    wire->write(addr);
    wire->endTransmission();
    wire->requestFrom(i2cAddress, 1);
    return wire->read();
}


/**
 * Writes a given register
 */
void myMcp23017Impl::writeRegister(int regAddr, int regValue)
{
    // Write the register
    uint8_t txBuffer[2]={regAddr,regValue};
    wire->beginTransmission(i2cAddress);
    wire->write(txBuffer,2);
    wire->endTransmission();
}


// EOF


