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


class myMcp23017 
{
public:
            myMcp23017(uint8_t addr=0, WireBase *w=NULL);
  void      begin(void);

  void      pinMode(uint8_t p, uint8_t d);
  void      digitalWrite(uint8_t p, uint8_t d);
  void      pullUp(uint8_t p, uint8_t d);
  uint8_t   digitalRead(uint8_t p);

  void      writeGPIOAB(uint16_t);
  uint16_t  readGPIOAB();
  uint8_t   readGPIO(uint8_t b);

  void      setupInterrupts(uint8_t mirroring, uint8_t open, uint8_t polarity);
  void      setupInterruptPin(uint8_t p, uint8_t mode);
  uint8_t   getLastInterruptPin();
  uint8_t   getLastInterruptPinValue();

 private: 

  uint8_t   bitForPin(uint8_t pin);
  uint8_t   regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr);

  uint8_t   readRegister(uint8_t addr);
  void      writeRegister(uint8_t addr, uint8_t value);

  /**
   * Utility private method to update a register associated with a pin (whether port A/B)
   * reads its value, updates the particular bit, and writes its value.
   */
  void      updateRegisterBit(uint8_t p, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr);

protected:
    WireBase *wire;
    int      i2cAddress;
  
};
