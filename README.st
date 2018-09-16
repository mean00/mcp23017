
This is a derivative from the Adafruit MCP23017 i2c expander board
The main differences are :

* Axx are all inputs
* Bxx are all outputs
* Built in support for rotary encoders
* Build in support for input lines
* Can use hw i2c or sw i2c

Since it is an i2c connected device, it is not a great idea to use i2c under interrupts.
As a result, it is half polling/half interrupt.

You MUST call the process function regularily so that the pending interrupt is processed completely

Usage :

myMcpButtonInput  *mcp= myMcp23017::create(PA2);  <= This is the  interrupt pin, PA2 on STM32 duino

  pushButton=new myMcpButtonInput(mcp,2); // Create a push button connected to pin A2
  rotarty= new myMcpRotaryEncoder(mcp,3,4); // create a rotarty encoder connected to A3 & A4

 mcp->start(); // Once all the clients have been added, start the engine

You just have to call mcp->process() often and it will work
