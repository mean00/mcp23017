/**
    This is a library for the MCP23017 i2c port expander
    It is derived from the adafruit one with the following change :
        * All Axxx port are inputs
        * All Bxxx ports are outputs
        * Additional methods to easily manage rotarty encoder and input button
 * 
 * It is using ARM friendly convention i.e. using "int" as much as possible
 * (c) mean00, BSD License
 
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
#include <mcp23017.h>

class mcpClientInfo
{
public:
        int         mask;
        myMcpClient *client;
};

/**
 * 
 * @param pinInterrupt
 * @param addr
 * @param w
 */
class myMcp23017Impl: public myMcp23017
{
public:
            myMcp23017Impl(uint8_t addr=0, WireBase *w=NULL);
  void      interrupt();
  /**
   * Call this frequently so that the internal events are processed
   * It is a bad idea to do i2c under interrupt....
   * 
   */
  void      process();
  
  /*        
   *   Once you have set it up completely, call this
   */
  void      start();

  
        /**
         * \fn digitalWrite
         * @param pin 0..7 matching b0...b7
         * @param value
         */
  void      digitalWrite(int pin, bool onoff);

  void      registerClient(int mask, myMcpClient *client);

 private: 
  uint8_t   readRegister(int addr);
  void      writeRegister(int addr, int value);
   
  
protected:
    void      init(void);
    WireBase *wire;
    int      i2cAddress;
    int      pinInterrupt;
    int      inputMask;
    int      PortALatch;
    int      PortBValue;
    bool     changed;
    mcpClientInfo clients[10];
    int      nbClients;
  
};
