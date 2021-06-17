#include "MyAvr.hpp"
#include "Twim.hpp"
#include "Ds3231.hpp"
#include "Printer.hpp"

Twim twim;
Ds3231 ds3231{ twim };

static void togPC3(){ DDRC |= 1<<3; PINC |= 1<<3; } //togle PC3

#if 0
int main(){

    CLKPR = 0x80; CLKPR = 0; //1MHz -> 8MHz in case fused to 1MHz

    sei();

    u8 seconds = 0;
    u8 secondsPrev = 0;

    while(1){
        while( not ds3231.seconds(seconds) or seconds == secondsPrev ){ _delay_ms(100); }
        secondsPrev = seconds;
        togPC3(); //toggle every second
    }

}
#else //callback version

#include "Uart.hpp"
Uart debug;

//callback from ds3231.update()
//callbacks from twi provide the bool result and have already sent a stop
static void twimCallback(bool ok){
    static u8 secondsPrev;
    if( not ok ) return; //failed, data no good
    auto s = ds3231.seconds(); //ok, get seconds
    if( s == secondsPrev ) return; //same as before?
    secondsPrev = s; //save
    togPC3(); //toggle every second
    //send debug output (uart)
    debug << setwf(2,'0') << ds3231.minutes() << ':' << setw(2) << s << endlc;
}

int main(){

    CLKPR = 0x80; CLKPR = 0; //1MHz -> 8MHz in case fused to 1MHz

    sei();



    ds3231.clear();

    while(1){
        //start a read of all registers, call twimCallback when done
        while( not ds3231.update(twimCallback) ){} //if twi busy, keep trying
        //do every 100ms
        _delay_ms(100);
    }

}
#endif
