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
    // Reset the chip, connected to PA1
    pinMode(PA1,OUTPUT);
    digitalWrite(PA1, LOW); 
    delay(100);
    digitalWrite(PA1, HIGH); 
    
    //
    
    
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
    static int blk=0;
    
    for(int i=0;i<10;i++)
    {
        mcp->process();
        delay(10);
    }
    if(pushButton->changed())
    {
        pin=!pin;
        mcp->digitalWrite(0,pin);
    }
    mcp->digitalWrite(7,blk&4);
    blk++;
    
    
}
//-