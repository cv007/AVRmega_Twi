#include "MyAvr.hpp"
#include "Twim.hpp"

//if know the name created elsewhere
//extern Twim& twim;
//ISR(TWI_vect){ twim.isr(); }

//or if not, there is only 1 twi in this mcu, and is static, can call directly
ISR(TWI_vect){ Twim::isr(); }


