#pragma once //Twim.hpp

#include "MyAvr.hpp"

/*-------------------------------------------------------------
    TWCR info

                --TWCR--
                TWINT TWEA TWSTA TWSTO TWWC TWEN - TWIE
    START         1    x     1     0     0   1   0   1  0xA5
    STOP          1    x     0     1     0   1   0   0  0x94
    ack           1    1     0     0     0   1   0   1  0xC5
    nack          1    0     0     0     0   1   0   1  0x85

    ack/nack are for read
    write can use either ack or nack to keep going since either are unused
    (ack used in this code when writing)

    TWSR bits 7-3, bits 1:0 is TWPS prescaler
    hex value is with bits in register bit position, decimal number is value
    shifted right by 3, code will shift TWSR right by 3 to get the decimal
    status value as its easier to deal with these decimal numbers and the
    prescale bits need to be removed in any case (mask vs shift)

    1   0x08 = start
    2   0x10 = repeated start
    3   0x18 = addressW+ack
    4   0x20 = addressW+nack
    5   0x28 = write+ack
    6   0x30 = wrote+nack
    7   0x38 = arb lost
    8   0x40 = addressR+ack
    9   0x48 = addressR+nack
    10  0x50 = read+ack
    11  0x58 = read+nack
--------------------------------------------------------------*/


/*-------------------------------------------------------------
    Twim - Twi Master
--------------------------------------------------------------*/
class Twim {

//-----------
    public:
//-----------

                using
callbackT       = void(*)(bool); //bool = lastResultOK_

                enum
SPEED           { KHZ100 = 100000ul, KHZ400 = 400000ul };

                //ACK = ack next rx (ACK will also be used used for write)
                //NACK = nack next rx
                //STOP = stop,nack,irq off
                enum
CMD             { START = 0xA5, STOP = 0x94, ACK = 0xC5, NACK = 0x85  };


                static auto
off             (){ TWCR = 0; }

                static auto //will also reset twi (off)
address         (uint8_t addr){ off(); addr_ = addr<<1; }

                static auto
callback        (callbackT cb){ callback_ = cb; }

                //will be set to 100kHz if no one specifies
                static void //(fcpu/speed/2)-8, 16MHz- 100kHz = 72, 400kHz = 12
speed           (SPEED e = KHZ100) { TWBR = F_CPU/e/2 -8; }

                static bool
lastResultOK    (){ return lastResultOK_; }

                static bool
isBusy          (){ return TWCR bitand 1; } //TWIE == 1 ?

                //TWSR, public if someone wants to take a look
                static auto
status          (){ return TWSR>>3; } //shifted to 0-31

                //blocking wait, with timeout in us
                static auto
waitUS          (uint16_t us)
                {
                while( isBusy() and us-- ) _delay_us(1);
                return lastResultOK_;
                //if false, can check isBusy() to see if twi is still active
                }

//the main write/read functions

                //write+read, pointers+len (all versions end up here, except writeWrite)
                static void
writeRead       (const uint8_t* wbuf, const uint8_t wlen, uint8_t* rbuf, const uint8_t rlen)
                {
                wbuf_ = wbuf;
                //make sure wbuf is not 0 before setting the end address
                //in case someone sneaks in a wlen >0 when wbuf was 0
                //(of course no way to check if wlen is actually valid, unless
                // the template versions below are used when length is in the type)
                wbufEnd_ = wbuf ? wbuf+wlen : wbuf;
                rbuf_ = rbuf;
                //same check as abive
                rbufEnd_ = rbuf ? rbuf+rlen : rbuf;
                wbuf2_ = 0;
                wbuf2End_ = 0;
                startIrq();
                }

                //write+write (seperate buffers, both writing)
                static void
writeWrite      (const u8* wbuf, const u8 wlen, const u8* wbuf2, const u8 wlen2)
                {
                wbuf_ = wbuf;
                wbufEnd_ = wbuf ? wbuf+wlen : wbuf;
                wbuf2_ = wbuf2;
                wbuf2End_ = wbuf2 ? wbuf2+wlen2 : wbuf2;
                rbuf_ = 0;
                rbufEnd_ = 0;
                startIrq();
                }



                //write+read, len included in type
                template<uint8_t NW, uint8_t NR>
                static void
writeRead       (uint8_t (&wbuf)[NW], uint8_t (&rbuf)[NR])
                {
                writeRead( wbuf, NW, rbuf, NR );
                }


                //write+write, len included in type (seperate buffers, both writing)
                template<unsigned N1, unsigned N2>
                static void
writeWrite      (const u8 (&wbuf)[N1], const u8 (&wbuf2)[N2])
                {
                writeWrite( wbuf, N1, wbuf2, N2 );
                }


                //write only
                static void
write           (uint8_t* wbuf, uint8_t wlen)
                {
                writeRead( wbuf, wlen, 0, 0 );
                }

                //write only, len included in type
                template<uint8_t N>
                static void
write           (uint8_t (&wbuf)[N])
                {
                writeRead( wbuf, N, 0, 0 );
                }

                //read only
                static void
read            (uint8_t* rbuf, uint8_t rlen)
                {
                writeRead( 0, 0, rbuf, rlen );
                }

                //read only, len included in type
                template<uint8_t N>
                static void
read            (uint8_t (&rbuf)[N])
                {
                writeRead( 0, 0, rbuf, N );
                }

//called from TWI_vect isr
                static void
isr             ()
                {
                switch( status() ) {
                    case 1: //start done
                    case 2: //repeated start done
                         //if wbuf has data, write, else read
                        TWDR = (wbuf_ < wbufEnd_ ? addr_ : addr_ bitor 1);
                        ack();
                        break;
                    case 3: //addressW was ack'd
                    case 5: //write ack'd
                        if( wbuf_ < wbufEnd_ ){ TWDR = *wbuf_++; return ack(); } //write data
                        if( wbuf2_ < wbuf2End_ ){ TWDR = *wbuf2_++; return ack(); } //write data
                        if( rbuf_ < rbufEnd_ ) return start(); //switch to read via repeated start if we have data to read
                        //did not switch to read, must be done (was write only)
                        lastResultOK_ = true;
                        stop(); //no more data
                        break;
                    case 8: //addressR was ack'd
                        ack(); //keep going, ack next read
                        break;
                    case 10: //read ack'd (by us)
                    case 11: //read nack'd (by us)
                        *rbuf_++ = TWDR;
                        if( rbuf_ >= rbufEnd_ ){ //if no more
                            lastResultOK_ = true;
                            stop(); //we already nack'd, now need stop
                            return;
                            }
                        rbuf_ < (rbufEnd_-1) ? ack() : nack();
                        break;
                    //case 4: //addressW was nack'd
                    //case 9: //addressR was nack'd
                    //case 6: //write nack'd
                    default: //arb lost or unknown code
                        stop();
                    }
                }
//-----------
    private:
//-----------
                //start a transaction (called by writeRead, writeWrite)
                static void
startIrq        ()
                {
                pinsInit();
                lastResultOK_ = false;
                if( not TWBR ) speed(); //if not set, set to default speed
                start();
                }

                static auto
nack            () -> void { TWCR = NACK; }

                static auto
start           () -> void { TWCR = START; }

                static auto
stop            () -> void
                {
                TWCR = STOP;
                wbuf_ = 0;
                wbufEnd_ = 0;
                wbuf2_ = 0;
                wbuf2End_ = 0;
                rbuf_ = 0;
                rbufEnd_ = 0;
                if( callback_ ) callback_( lastResultOK_ );
                }

                static auto
ack             () -> void { TWCR = ACK; }

                //set scl/sda pins - PC4/SDA, PC5/SCL
                //add pullup in case no external pullups are being used
                static auto
pinsInit        () -> void
                {
                DDRC and_eq compl ((1<<4) bitor (1<<5)); //input
                PORTC or_eq ((1<<4) bitor (1<<5)); //pullup
                }

                static inline uint8_t addr_;
                static inline bool lastResultOK_;
                static inline const uint8_t* wbuf_;
                static inline const uint8_t* wbufEnd_;
                static inline const uint8_t* wbuf2_;
                static inline const uint8_t* wbuf2End_;
                static inline uint8_t* rbuf_;
                static inline uint8_t* rbufEnd_;
                static inline callbackT callback_;

};



