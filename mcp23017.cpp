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

static void _myInterrupt(void *cookie)
{
    myMcp23017Impl *mcp=(myMcp23017Impl *)cookie;
    mcp->interrupt();
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
        nbClients=0;
        inputMask=0;
        
}

/**
 * \fn start
 * \brief start the interrupt system
 */
void myMcp23017Impl::start()
{
    // All interrupt on change for all A
    writeRegister(MCP23017_GPINTENA,inputMask);
    // And use previous value
    writeRegister(MCP23017_INTCONA,0);
    // Clear pending interrupts if any
    readRegister(MCP23017_GPIOA);
    readRegister(MCP23017_GPIOB);
    noInterrupts();        
    pinMode(pinInterrupt,INPUT_PULLUP);
    attachInterrupt(pinInterrupt,_myInterrupt,this,FALLING);    
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
    // Ok an interrupt occurred, process it 
    
    // MCP23017_INTCAPA = value when interrupt occurred
    // MCP23017_GPIOA = value now
    int newValue=readRegister(MCP23017_INTCAPA) ; //MCP23017_INTCAPA);
    if(newValue!=PortALatch)
    {
        int changed=newValue^PortALatch;
        for(int i=0;i<nbClients;i++)
        {
            int masked=clients[i].mask & changed;
            if(masked)
            {
                clients[i].client->process(masked,newValue);
                changed &=~masked;
            }
            if(!changed)
                break;
        }
    }
    PortALatch=newValue;
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

//--
void      myMcp23017Impl::registerClient(int mask, myMcpClient *client)
{
    mcpClientInfo *slot=clients+nbClients;
    nbClients++;
    
    slot->client=client;
    slot->mask=mask;
    inputMask|=mask; // Allow interrupts on those pins
}
//
//  Input button
//
//
//
/**
 * 
 * @param mcp
 * @param pin
 */

myMcpButtonInput::myMcpButtonInput(myMcp23017 *mcp, int pin) : myMcpClient(mcp)
{
          _pin=pin;
          _state=false;
          _changed=false;
          myMcp23017Impl *impl=(myMcp23017Impl *)mcp;
          impl->registerClient(1<<pin,this);
}
/**
 * 
 * @param pins
 * @param states
 * @return 
 */
 bool myMcpButtonInput::process(int pins, int states)
 {
     int oldstate=_state;
     _state=!!(states & (1<<_pin));
     _changed=(_state != oldstate);
     return true;
 }
//
//
//
//
 
 // Use the full-step state table (emits a code at 00 only)
#define R_START     0x0
#define R_CW_FINAL  0x1
#define R_CW_BEGIN  0x2
#define R_CW_NEXT   0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT  0x6
 // Directions
#define DIR_NONE    0x0
// Clockwise step.
#define DIR_CW      0x10
// Counter-clockwise step.
#define DIR_CCW     0x20
 
const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
 
 /**
  * 
  * @param mcp
  * @param pin1
  * @param pin2
  */
 myMcpRotaryEncoder::myMcpRotaryEncoder(myMcp23017 *mcp, int pin1,int pin2): myMcpClient(mcp)
 {
     _pin1=pin1;
     _pin2=pin2;
     myMcp23017Impl *impl=(myMcp23017Impl *)mcp;
     impl->registerClient((1<<pin1)+(1<<pin2),this);
     _state = R_START;
     _count=0;
 }
/**
 * 
 * @param pins
 * @param states
 * @return 
 */
bool myMcpRotaryEncoder::process(int pins, int states)
 {
    int s2=!!(states & (1<<_pin2));
    int s1=!!(states & (1<<_pin1));
    
    int pinstate=(s2<<1) | s1;
     
    // Determine new state from the pins and state table.
    _state = ttable[_state & 0xf][pinstate];
    // Return emit bits, ie the generated event.
    switch(_state & 0x30)
    {
        case DIR_CW:  _count++;break;
        case DIR_CCW: _count--;break;
    }
    return true;
 }

// EOF


