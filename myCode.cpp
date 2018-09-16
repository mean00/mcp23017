/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "mcp23017.h"

myMcp23017 *mcp;

myMcpButtonInput *pushButton;

void mySetup() 
{
    Wire.begin();
    Serial.begin(57600);
    Serial.println("Init"); 
    mcp= myMcp23017::create(PA2);
    // continue init here, add clients
    pushButton=new myMcpButtonInput(mcp,2); // A2
    
    
    // then go
    mcp->start();
    
}
/**
 */



void myLoop(void) 
{
    static bool pin=false;
    
    mcp->process();
    if(pushButton->changed())
    {
        pin=!pin;
        mcp->digitalWrite(0,pin);
    }
    delay(100);
    
}
//-