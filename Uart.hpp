#pragma once //Uart.hpp

#include "MyAvr.hpp"
#include "Printer.hpp"

/*-------------------------------------------------------------
    Uart - simple tx only - 8MHz/250kbaud, PD1 is TX
--------------------------------------------------------------*/
class Uart : public Printer {

public:

Uart            (){ UCSR0B = 1<<3; UBRR0 = 1; } //txen, 250kbaud

private:

                virtual bool
write           (const char c)
                {
                while( not (UCSR0A bitand (1<<5)) ){} //UDRE0
                UDR0 = c;
                return true;
                }

};
