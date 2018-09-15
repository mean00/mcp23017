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
    mcp=new myMcp23017();
    mcp->begin();
}
/**
 */



void myLoop(void) 
{
    static uint16_t val;
    val=mcp->readGPIOAB();
    Serial.print("Val");
    Serial.println(val);
    delay(500);

}
//-