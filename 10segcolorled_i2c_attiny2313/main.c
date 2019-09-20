/*
 * 10segcolorled_i2c_attiny2313.c
 *
 * Created: 2019/09/17 22:33:41
 * Author : rimksky@gmail.com
 */ 

/*
    ATtiny2313 fuse factory default LOW:0x64 HIGH:0xDF EXT:0xFF LOCK:0xFF
    I2C Address(Default): 0x55

    port mapping [ATtiny2313]

    bit      7    6    5    4    3    2    1    0
    PORTA    -    -    -    -    -    -    -    -
    PORTB    -    r    -    b    g    J    I    H
    PORTD    -    G    F    E    D    C    B    A

    10seg fullcolor [OSX10201-LRPB2]

    ABCDEFJHIJ (VCC)
    rbg----gbr (GND) -> The side where the string is printed

    r - 150R - GND
    b - 100R - GND
    g - 100R - GND
*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

//#define DEBUG_INIT
#define SLAVE_ADDRESS 0x55

#include "usiTwiSlave.h"

#ifdef DEBUG_INIT
uint8_t i2c_register[] =
{
    0b001,  // 0x01 A
    0b010,  // 0x02 B
    0b011,  // 0x03 C
    0b100,  // 0x04 D
    0b101,  // 0x05 E
    0b110,  // 0x06 F
    0b111,  // 0x07 G
    0b001,  // 0x08 H
    0b010,  // 0x09 I
    0b011,  // 0x0a J
};
#else
uint8_t i2c_register[] =
{
    0b000,  // 0x01 A
    0b000,  // 0x02 B
    0b000,  // 0x03 C
    0b000,  // 0x04 D
    0b000,  // 0x05 E
    0b000,  // 0x06 F
    0b000,  // 0x07 G
    0b000,  // 0x08 H
    0b000,  // 0x09 I
    0b000,  // 0x0a J
};
#endif
const uint8_t i2c_register_size = sizeof(i2c_register);


/******************************************************************************
 * Initialize CPU
 ******************************************************************************/
void init_cpu(void)
{
    // Clock Prescale Register
    CLKPR = 0b10000000; // CLKPCE=enable,  CLKPS3,2,1,0=clear
    CLKPR = 0b00000011; // CLKPCE=disable, CLKPS3,2,1,0=0b0011(1/8)

    // Port Data Direction Register (10seg LED : Output)
    DDRB  |= (_BV(DDB0) | _BV(DDB1) | _BV(DDB2) | _BV(DDB3) | _BV(DDB4)             | _BV(DDB6));
    DDRD  |= (_BV(DDD0) | _BV(DDD1) | _BV(DDD2) | _BV(DDD3) | _BV(DDD4) | _BV(DDD5) | _BV(DDD6));

    // Port Data Register (10seg LED bar switch : Active-Low)
    PORTB &=  ~(_BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2));
    PORTD &=  ~(_BV(PORTD0) | _BV(PORTD1) | _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6));

    // Port Data Register (10seg LED color switch : Active-High)
    PORTB |= (_BV(PORTB3) | _BV(PORTB4) | _BV(PORTB6));

    // Timer/Counter
    TCCR0A = _BV(WGM01);        // Timer/Counter0 Control Register A
    TCCR0B = _BV(CS00);         // Timer/Counter0 Control Register B
    TCNT0 = 0;                  // Timer/Counter0
    OCR0A = 233;                // Timer/Counter0 Output Compare A Register A
                                //  => 8MHz/CLKPS(8)/CS(1)/223=4.48kHz (LED refresh is 560Hz)
    OCR0B = 0;                  // Timer/Counter0 Output Compare B Register B
                                //  => don't use.
    TIMSK = _BV(OCIE0A);        // Timer/Counter Interrupt Mask Register
                                // => Output Compare Match A Interrupt Enable

    return;
}

void led_off(){
    // Port Data Register (10seg LED bar switch : Active-Low)
    PORTB &= ~(_BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2));
    PORTD &= ~(_BV(PORTD0) | _BV(PORTD1) | _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6));

    // Port Data Register (10seg LED color switch : Active-High)
    PORTB |= (_BV(PORTB3) | _BV(PORTB4) | _BV(PORTB6));
}

void flush_led( uint16_t posbit, uint8_t colorbit ){
    uint8_t tmpb, tmpd;
    tmpb = PORTB;
    tmpd = PORTD;

    tmpb = ( 0b10100000 & tmpb ) |
    ( 0b00000111 & ( posbit >> 7 ) ) |
    ( ( ( 0b00011000 ) & ( colorbit << 3 ) ) ^ 0b00011000 ) |
    ( ( ( 0b01000000 ) & ( colorbit << 4 ) ) ^ 0b01000000 );
    tmpd = ( 0b10000000 & tmpd ) | ( 0b01111111 & posbit );

    PORTB = tmpb;
    PORTD = tmpd;
}

void reflesh_led(void)
{
    static uint8_t seg = 0;
    uint16_t posbit;
    uint8_t colorbit;
    led_off();
    posbit = ( 1 << seg );
    colorbit = i2c_register[seg];
    flush_led( posbit, colorbit );
    seg++;
    if( seg >= i2c_register_size ){
        seg = 0;
    }
    return;
}

ISR(TIMER0_COMPA_vect)
{
    reflesh_led();
    return;
}

int main(void)
{
    uint8_t rx[TWI_RX_BUFFER_SIZE];
    uint8_t rx_size = 0;
    uint8_t is_read_register = 0;
    uint8_t read_register = 0;
    uint8_t r, r1, r2;
    uint8_t i;

    init_cpu();
    usiTwiSlaveInit(SLAVE_ADDRESS);
    sei();

    while (1) 
    {
        if (usiTwiDataInReceiveBuffer()){
            is_read_register = 0;
            r = 0;
            rx_size = 0;

            // pull rx buffer
            for( i = 0; i < TWI_RX_BUFFER_SIZE; i++ ){
                if (usiTwiDataInReceiveBuffer()){
                    rx_size++;
                    rx[i] = usiTwiReceiveByte();
                }
            }

            // analyze rx buffer
            while( rx_size > r ){
                // write register (2 byte)
                // recv: [addr] [value]
                if( ( rx_size - r ) >= 2 ){
                    r1 = rx[r++];
                    r2 = rx[r++];
                    if( r1 < i2c_register_size ){
                        i2c_register[r1] = r2 & 0b0000111;
                    }
                    else if( r1 == 0x10 ){
                           for(i = 0; i < i2c_register_size; i++ ){
                            i2c_register[i] = r2 & 0b0000111;
                        }
                    }
                    else if( r1 == 0x11 ){
                        for(i = i2c_register_size-1; i > 0; i-- ){
                            i2c_register[i] = i2c_register[i-1] & 0b0000111;
                        }
                        i2c_register[i] = r2 & 0b0000111;
                    }
                    else if( r1 == 0x12 ){
                        for(i = 0; i < i2c_register_size-1; i++ ){
                            i2c_register[i] = i2c_register[i+1] & 0b0000111;
                        }
                        i2c_register[i] = r2 & 0b0000111;
                    }
                }

                // set read register (1 byte)
                // recv: [addr]  -> send: [value]
                else if( ( rx_size - r ) == 1 ){
                    r1 = rx[r++];
                    if( r1 < i2c_register_size ){
                        is_read_register = 1;
                        read_register = r1;
                    }
                }
            }
        }
        // push tx buffer
        if( is_read_register ){
            usiTwiTransmitByte( i2c_register[read_register] );
            is_read_register = 0;
        }
    }
    return (0);
}
