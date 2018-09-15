/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "mcp23017.h"

myMcp23017 *mcp;

void mySetup() 
{
    Wire.begin();
    Serial.begin(57600);
    Serial.println("Init"); 
    mcp=new myMcp23017(PA2);
    // continue init here
    
    // then go
    mcp->start();
    
}
/**
 */



void myLoop(void) 
{
#if 0
    static uint16_t val;
    mcp->process();
    delay(10);
#else
    static int pin=0;
    
    pin++;
    mcp->digitalWrite(pin%8,!!(pin&16));
    mcp->process();
    delay(100);
    
#endif
}
//-