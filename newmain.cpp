#include "MyAvr.hpp"
#include "Twim.hpp"
#include "Ds3231.hpp"

Twim twim;
Ds3231 ds3231{ twim };

static void togPC3(){ DDRC |= 1<<3; PINC |= 1<<3; } //togle PC3

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

