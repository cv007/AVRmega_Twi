## C++ Twi driver for ATmega168 and its siblings

#### tested on an ATmega168, using the example in main
----------

**usage**

**first create an Isr.cpp file, which is simple**
```
#include "MyAvr.hpp"
#include "Twim.hpp"
ISR(TWI_vect){ Twim::isr(); }
```
**then in whatever source file you need to use twi (will typically be like the Ds3231.hpp header)**
```
#include "MyAvr.hpp" //some universal things everyone should include
#include "Twim.hpp" //header only

Twim twim; //create instance, or can skip since it is all static functions
           //example- Twim::address(0x68) is same as twim.address(0x68)

twim.address( 0x68 ); //set address, remains in use until changed
//if interrupts not on, they will need to be one
sei(); //MyAvr.hpp has the include needed for this

u8 wbuf[1] = { 0 }; //a write buffer which has the write data (if writing)
u8 rbuf[19]; //a read buffer (if reading)

//start a transaction in several ways-
twim.writeRead( wbuf, rbuf ); //we want all the buffer used, so the sizes are already in the type
twim.writeRead( wbuf, 1, rbuf, 1 ); //or we can specify sizes if less than all the buffere needed
//there are also write only and read only versions

//the interrupt process is now started, and we need to either wait
//or do something else end check back later

bool ok = twim.waitUS( 3000 ); //can use the blocking wait in Twim
if( ok ){ //no timeout, lastResultOK() was true
    /* we have rbuf filled */ 
}
else { //timeout, or lastResultOK() was false
    /* can check isBusy() to see if was timeout, 
       if so, may want to twim.off() to reset twi */ 
}
````

**The code is simple, so if you want to change it, change it to your liking.**

