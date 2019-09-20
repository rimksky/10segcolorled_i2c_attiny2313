/*********************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Slave.h
* Compiler          : IAR EWAAVR 4.11A
* Revision          : $Revision: 1.14 $
* Date              : $Date: Friday, December 09, 2005 17:25:38 UTC $
* Updated by        : $Author: jtyssoe $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*                     The example is written for the ATmega169, ATtiny26 & ATtiny2313
*
* AppNote           : AVR312 - Using the USI module as a TWI slave
*
* Description       : Header file for USI_TWI driver
*
* History           :
*    2017/06     modified by @jsdiy http://jsdiy.webcrow.jp/
*    2017/09     modified by @jsdiy deleted usiTwiAvrDef.h and imported AVR port definition into usiTwiSlave.h.
*    2019/09     modified by @rimksky formatted text.
*********************************************************************/

#ifndef USITWISLAVE_H_
#define USITWISLAVE_H_

//! Prototypes
void usiTwiSlaveInit( uint8_t );
void usiTwiTransmitByte( uint8_t );
uint8_t usiTwiReceiveByte( void );
uint8_t usiTwiDataInReceiveBuffer( void );

//////////////////////////////////////////////////////////////////
///////////////// Driver Buffer Definitions //////////////////////
//////////////////////////////////////////////////////////////////
// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes
//    Change according to the nature of the slave device
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE 32
#endif
#define TWI_RX_BUFFER_MASK ( TWI_RX_BUFFER_SIZE - 1 )

#if ( TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK )
#error TWI RX buffer size is not a power of 2
#endif

// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes
//    Change according to the nature of the slave device
#ifndef TWI_TX_BUFFER_SIZE
#define TWI_TX_BUFFER_SIZE 16
#endif
#define TWI_TX_BUFFER_MASK ( TWI_TX_BUFFER_SIZE - 1 )

#if ( TWI_TX_BUFFER_SIZE & TWI_TX_BUFFER_MASK )
#error TWI TX buffer size is not a power of 2
#endif


// AVR port definition
#if defined(__AVR_ATtiny26__)
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB0
#define PORT_USI_SCL        PORTB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_STRT_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__AVR_ATtiny2313__) | \
    defined(__AVR_ATtiny2313A__) | defined(__AVR_ATtiny4313__)
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB5
#define PORT_USI_SCL        PORTB7
#define PIN_USI_SDA         PINB5
#define PIN_USI_SCL         PINB7
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#endif

#if defined(__AVR_ATtiny44__) | defined(__AVR_ATtiny44A__) | \
    defined(__AVR_ATtiny84__)
#define DDR_USI             DDRA
#define PORT_USI            PORTA
#define PIN_USI             PINA
#define PORT_USI_SDA        PORTA6
#define PORT_USI_SCL        PORTA4
#define PIN_USI_SDA         PINA6
#define PIN_USI_SCL         PINA4
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_STR_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__AVR_ATtiny25__) | \
    defined(__AVR_ATtiny45__) | \
    defined(__AVR_ATtiny85__)
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PB0
#define PORT_USI_SCL        PB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__AVR_ATmega165__) | defined(__AVR_ATmega169PA__) |\
    defined(__AVR_ATmega325__) | defined(__AVR_ATmega3250__) | \
    defined(__AVR_ATmega645__) | defined(__AVR_ATmega6450__) | \
    defined(__AVR_ATmega329__) | defined(__AVR_ATmega3290__) | \
    defined(__AVR_ATmega649__) | defined(__AVR_ATmega6490__)
#define DDR_USI             DDRE
#define PORT_USI            PORTE
#define PIN_USI             PINE
#define PORT_USI_SDA        PORTE5
#define PORT_USI_SCL        PORTE4
#define PIN_USI_SDA         PINE5
#define PIN_USI_SCL         PINE4
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#endif

#if defined(__AVR_ATmega169__)
#define DDR_USI             DDRE
#define PORT_USI            PORTE
#define PIN_USI             PINE
#define PORT_USI_SDA        PORTE5
#define PORT_USI_SCL        PORTE4
#define PIN_USI_SDA         PINE5
#define PIN_USI_SCL         PINE4
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_STRT_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#endif    //USITWISLAVE_H_
