#pragma once //Ds3231.hpp

#include "MyAvr.hpp"
#include "Twim.hpp"

/*-------------------------------------------------------------
    Ds3231
--------------------------------------------------------------*/
class Ds3231 {

                enum { SLAVE_ADDRESS = 0x68 };
                enum { US_TIMEOUT = 2000 };

                //normally would have a struct of all register bits,
                //but just have seconds here for a simple example
                using
registersT      = union {
                    u8 all[0x12+1]; //all registers, 0x00-0x12
                    struct {
                        //0x00
                        u8 seconds1  : 4;
                        u8 seconds10 : 4;
                        //0x01, etc.
                    };
                };

                registersT registers_;
                Twim& twim_;

                auto
readAll         ()
                {
                twim_.address( SLAVE_ADDRESS );
                //reg address start 0, will reuse the read buffer, which works ok
                //in this case (buffer is private, and only we can read it)
                registers_.all[0] = 0;
                twim_.writeRead( registers_.all, 1, registers_.all, sizeof registers_.all );
                return twim_.waitUS( US_TIMEOUT );
                }

//-----------
    public:
//-----------

Ds3231          (Twim& twim)
                : twim_(twim) {}

                //blocking, with timeout
                auto
seconds         (u8& seconds)
                {
                if( ! readAll() ) return false;
                seconds = registers_.seconds10 * 10 + registers_.seconds1;
                return true;
                }

};



