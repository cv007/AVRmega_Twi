## C++ Twi driver for ATmega168 and its siblings

#### tested on an ATmega168, using the example in main.cpp (included screenshot is the result)

#### I have not used these older avr's in quite a while, and do not plan on using them, so this code was meant to only show a way to use the twi peripheral that is most likely more simple than other versions. The mega168 will go back in storage, and this repository will most likely not be updated.
----------

**--usage--**

**You will need a C++17 capable compiler (gcc-avr 7.3.0 from Arduino will do), as the Twim.hpp header has static variables in the class header. Add the g++ option -std=c++17. It is not a requirement that the class functions/vars be static, but is a little simpler when you only have the capbility to use one instance anyway (only 1 twi peripheral), such as calling the isr() function from the twi_vect isr function will not need an instance.**

**first create an Isr.cpp file, or place this in whichever source file you want**
```
#include "MyAvr.hpp"
#include "Twim.hpp"
ISR(TWI_vect){ Twim::isr(); }
```
**then in whatever source file you need to use twi (will typically be like the Ds3231.hpp header- although that is not a great example as it is mainly being used to test twi and is not being used as it should be)**
```
#include "MyAvr.hpp" //some universal things everyone should include
#include "Twim.hpp" //header only

Twim twim; //create instance, or can skip since it is all static functions
           //example- Twim::address(0x68) is same as twim.address(0x68)

twim.address( 0x68 ); //set address, remains in use until changed
//if interrupts not on, they will need to be on
sei(); //MyAvr.hpp has the include needed for this

u8 wbuf[1] = { 0 }; //a write buffer which has the write data (if writing)
u8 rbuf[19]; //a read buffer (if reading)

//start a transaction in several ways-
twim.writeRead( wbuf, rbuf ); //we want all the buffer used, so the sizes are already in the type
twim.writeRead( wbuf, 1, rbuf, 1 ); //or we can specify sizes if less than all the buffere needed
//there are also write only and read only versions, and also added a writeWrite version for when
//you want to write from 2 buffers (command(s), then data)

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
**You can also use a callback if wanted, which will occur after the transaction is done and the stop command was just sent. The callback is provided an argument with the lastResultOK_ value.**

```
void myCallback(bool ok){
    if( ok ) /* do something */
    else /* do something else */
}

twim.callback(myCallback);
//do normal twi things, the callback will be called at the next stop command with lastResultOK_ as the argument

```

**The code is simple, so if you want to change anything, then simply change it to suit your needs.**

