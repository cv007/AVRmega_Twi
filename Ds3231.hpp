#pragma once //Ds3231.hpp

#include "MyAvr.hpp"
#include "Twim.hpp"

/*-------------------------------------------------------------
    Ds3231
--------------------------------------------------------------*/
class Ds3231 {

                enum { SLAVE_ADDRESS = 0x68 };
                enum { US_TIMEOUT = 4000 };

                //normally would have a struct of all register bits,
                //but just have seconds here for a simple example
                using
registersT      = union {
                    u8 all[0x12+1]; //all registers, 0x00-0x12
                    struct {
                        //0x00
                        u8 seconds1  : 4;
                        u8 seconds10 : 4;
                        u8 minutes1  : 4;
                        u8 minutes10 : 4;
                        //0x01, etc.
                    };
                };

                registersT registers_;
                Twim& twim_;
                u8 regAddr_[1];

                auto
readAll         ()
                {
                twim_.address( SLAVE_ADDRESS );
                regAddr_[0] = 0;
                twim_.writeRead( regAddr_, registers_.all );
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

                auto
clear           ()
                { //clear all time registers
                twim_.address( SLAVE_ADDRESS );
                for( auto& r : registers_.all ) r = 0; //clear all buffer
                twim_.writeWrite( regAddr_, 1, registers_.all, 0x0D );
                return twim_.waitUS( US_TIMEOUT );
                }

//callback testing

                //read all registers, with callback to call when done
                auto
update          (Twim::callbackT cb)
                {
                if( twim_.isBusy() ) return false;
                twim_.callback( cb );
                twim_.address( SLAVE_ADDRESS );
                //reg address start 0, will reuse the read buffer, which works ok
                //in this case (buffer is private, and only we can read it)
                registers_.all[0] = 0;
                twim_.writeRead( registers_.all, 1, registers_.all, sizeof registers_.all );
                return true;
                }

                //assumes caller knows data is good
                auto
seconds         ()
                {
                return registers_.seconds10 * 10 + registers_.seconds1;
                }
                auto
minutes         ()
                {
                return registers_.minutes10 * 10 + registers_.minutes1;
                }
};



