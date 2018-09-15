/**
    This is a library for the MCP23017 i2c port expander
    It is derived from the adafruit one with the following change :
        * All Axxx port are inputs
        * All Bxxx ports are outputs
        * Additional methods to easily manage rotarty encoder and input button
 
 */

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
#pragma once
#include <Wire.h>

/**
 * 
 * @param pinInterrupt
 * @param addr
 * @param w
 */
class myMcp23017 
{
public:
            myMcp23017(int pinInterrupt,uint8_t addr=0, WireBase *w=NULL);
  void      interrupt();
  /**
   * Call this frequently so that the internal events are processed
   * 
   */
  void      process();
  void      start();

  
        /**
         * \fn digitalWrite
         * @param pin 0..7 matching b0...b7
         * @param value
         */
  void      digitalWrite(int pin, bool onoff);



 private: 

 
  uint8_t   readRegister(uint8_t addr);
  void      writeRegister(uint8_t addr, uint8_t value);

  
protected:
    void      init(void);
    WireBase *wire;
    int      i2cAddress;
    int      pinInterrupt;
    int      PortALatch;
    int      PortBValue;
    bool     changed;
  
};
