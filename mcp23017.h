/**
    This is a library for the MCP23017 i2c port expander
    It is derived from the adafruit one with the following change :
        * All Axxx port are inputs
        * All Bxxx ports are outputs
        * Additional methods to easily manage rotarty encoder and input button
 * 
 * It is using ARM friendly convention i.e. using "int" as much as possible
 * (c) mean00, BSD License
 * Datasheet :
 * http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf
 * 
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
        static myMcp23017 *create(int pinInterrupt, int i2cAdr=0, WireBase *wire=NULL);
  /**
   * Call this frequently so that the internal events are processed
   * It is a bad idea to do i2c under interrupt....
   * 
   */
  virtual void      process()=0;
  
  /*        
   *   Once you have set it up completely, call this
   */
  virtual void      start()=0;

  
        /**
         * \fn digitalWrite
         * @param pin 0..7 matching b0...b7
         * @param value
         */
  virtual void      digitalWrite(int pin, bool onoff)=0;

protected:
            myMcp23017()
            {
              
            }

  
};

class myMcpClient
{
public:
        myMcpClient(myMcp23017 *m)
        {
          mcp=m;
        }        
        virtual bool process(int pins,int state)=0;
protected:
        myMcp23017 *mcp;
};

/**
 */
class myMcpButtonInput : public myMcpClient
{
public:
        myMcpButtonInput(myMcp23017 *mcp, int pin) ;
        bool process(int pin, int state);
        bool changed()
        {
          bool c= _changed;
          _changed=false;
          return c;
        }
        bool state()
        {
          return _state;
        }
protected:
        int  _pin;
        bool _state;
        bool _changed;
  
};


/**
 */
class myMcpRotaryEncoder : public myMcpClient
{
public:
        myMcpRotaryEncoder(myMcp23017 *mcp, int pin1,int pin2) ;
        bool process(int pin, int state);
        int count()
        {
          return _count;
        }
protected:
        int  _pin1,_pin2;
        int  _count;
        int  _state;
};
