/*********************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Slave.c
* Compiler          : IAR EWAAVR 4.11A
* Revision          : $Revision: 1.14 $
* Date              : $Date: Friday, December 09, 2005 17:25:38 UTC $
* Updated by        : $Author: jtyssoe $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*
* AppNote           : AVR312 - Using the USI module as a I2C slave
*
* Description       : Functions for USI_TWI_receiver and USI_TWI_transmitter.
*
* History           :
*    2017/06     modified by @jsdiy http://jsdiy.webcrow.jp/
*    2019/09     modified by @rimksky Added SET_USI_TO_SEND_ACK() to improve stability
*********************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usiTwiSlave.h"

// Inline Functions
static    inline    void    SET_USI_TO_SEND_ACK()
{
    USIDR    =  0;                                              /* Prepare ACK                         */
    DDR_USI |=  (1<<PORT_USI_SDA);                              /* Set SDA as output                   */
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|  /* Clear all flags, except Start Cond  */
                (0x0E<<USICNT0);                                /* set USI counter to shift 1 bit. */
}

static    inline    void    SET_USI_TO_READ_ACK()
{
    DDR_USI &=  ~(1<<PORT_USI_SDA);                             /* Set SDA as intput */
    USIDR    =  0;                                              /* Prepare ACK        */
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|  /* Clear all flags, except Start Cond  */
                (0x0E<<USICNT0);                                /* set USI counter to shift 1 bit. */
}

static    inline    void    SET_USI_TO_TWI_START_CONDITION_MODE()
{
    USICR    =  (1<<USISIE)|(0<<USIOIE)|                        /* Enable Start Condition Interrupt. Disable Overflow Interrupt.*/
                (1<<USIWM1)|(0<<USIWM0)|                        /* Set USI in Two-wire mode. No USI Counter overflow hold.      */
                (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|            /* Shift Register Clock Source = External, positive edge        */
                (0<<USITC);
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|  /* Clear all flags, except Start Cond               */
                (0x0<<USICNT0);
}

static    inline    void    SET_USI_TO_SEND_DATA()
{
    DDR_USI |=  (1<<PORT_USI_SDA);                                  /* Set SDA as output                  */
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      /* Clear all flags, except Start Cond */
                (0x0<<USICNT0);                                     /* set USI to shift out 8 bits        */
}

static    inline    void    SET_USI_TO_READ_DATA()
{
    DDR_USI &= ~(1<<PORT_USI_SDA);                                  /* Set SDA as input                   */
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      /* Clear all flags, except Start Cond */
                (0x0<<USICNT0);                                     /* set USI to shift out 8 bits        */
}

// OverflowState
typedef    enum
{
    USI_SLAVE_CHECK_ADDRESS                = 0x00,
    USI_SLAVE_SEND_DATA                    = 0x01,
    USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA = 0x02,
    USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   = 0x03,
    USI_SLAVE_REQUEST_DATA                 = 0x04,
    USI_SLAVE_GET_DATA_AND_SEND_ACK        = 0x05
}
EOverflowState;

/* Static Variables */
static    uint8_t    TWI_slaveAddress;
static    volatile    EOverflowState    overflowState;

/* Local variables */
static uint8_t TWI_RxBuf[TWI_RX_BUFFER_SIZE];
static volatile uint8_t TWI_RxHead;
static volatile uint8_t TWI_RxTail;

static uint8_t TWI_TxBuf[TWI_TX_BUFFER_SIZE];
static volatile uint8_t TWI_TxHead;
static volatile uint8_t TWI_TxTail;

/* Flushes the TWI buffers */
static    void    flushTwiBuffers(void)
{
    TWI_RxTail = 0;
    TWI_RxHead = 0;
    TWI_TxTail = 0;
    TWI_TxHead = 0;
}

//********** USI_TWI functions **********//

/* Initialise USI for TWI Slave mode. */
void usiTwiSlaveInit( uint8_t ownAddress )
{
    flushTwiBuffers();

    TWI_slaveAddress = ownAddress;

    PORT_USI |=  (1<<PORT_USI_SCL);                                 // Set SCL high
    PORT_USI |=  (1<<PORT_USI_SDA);                                 // Set SDA high
    DDR_USI  |=  (1<<PORT_USI_SCL);                                 // Set SCL as output
    DDR_USI  &= ~(1<<PORT_USI_SDA);                                 // Set SDA as input
    USICR    =  (1<<USISIE)|(0<<USIOIE)|                            // Enable Start Condition Interrupt. Disable Overflow Interrupt.
                (1<<USIWM1)|(0<<USIWM0)|                            // Set USI in Two-wire mode. No USI Counter overflow prior
                                                                    // to first Start Condition (potentail failure)
                (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|                // Shift Register Clock Source = External, positive edge
                (0<<USITC);
    USISR    =    (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|        // Clear all flags
                (0x00<<USICNT0);                                                // reset overflow counter
}

/* Puts data in the transmission buffer, Waits if buffer is full. */
void usiTwiTransmitByte( uint8_t data )
{
    TWI_TxHead = ( TWI_TxHead + 1 ) & TWI_TX_BUFFER_MASK;         // Calculate buffer index.
    while ( TWI_TxHead == TWI_TxTail );                           // Wait for free space in buffer.
    TWI_TxBuf[TWI_TxHead] = data;                                 // Store data in buffer.
}

/* Returns a byte from the receive buffer. Waits if buffer is empty. */
uint8_t usiTwiReceiveByte( void )
{
    while ( TWI_RxHead == TWI_RxTail );
    TWI_RxTail = ( TWI_RxTail + 1 ) & TWI_RX_BUFFER_MASK;        // Calculate buffer index (Store new index)
    return TWI_RxBuf[TWI_RxTail];                                // Return data from the buffer.
}

/* Check if there is data in the receive buffer. */
uint8_t usiTwiDataInReceiveBuffer( void )
{
    return ( TWI_RxHead != TWI_RxTail );                 // Return 0 (FALSE) if the receive buffer is empty.
}

/*
 * Usi start condition ISR
 * Detects the USI_TWI Start Condition and intialises the USI
 * for reception of the "TWI Address" packet.
 */
ISR( USI_START_VECTOR )
{
    overflowState = USI_SLAVE_CHECK_ADDRESS;
    DDR_USI  &= ~(1<<PORT_USI_SDA);                                 // Set SDA as input
    while ( (PIN_USI & (1<<PORT_USI_SCL)) && !(PIN_USI & (1<<PORT_USI_SDA)) );   // Wait for SCL to go low to ensure the "Start Condition" has completed.

    USICR   =   (1<<USISIE)|(1<<USIOIE)|                            // Enable Overflow and Start Condition Interrupt. (Keep StartCondInt to detect RESTART)
                (1<<USIWM1)|(1<<USIWM0)|                            // Set USI in Two-wire mode.
                (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|                // Shift Register Clock Source = External, positive edge.
                                                                    // 4-Bit Counter Source = external, both edges.
                (0<<USITC);                                            // no toggle clock-port pin

    USISR  =    (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Clear flags
                (0x0<<USICNT0);                                     // Set USI to sample 8 bits i.e. count 16 external pin toggles.
}

/*
 * USI counter overflow ISR
 * Handels all the comunication. Is disabled only when waiting
 * for new Start Condition.
 */
ISR( USI_OVERFLOW_VECTOR )
{
    switch (overflowState)
    {
    // ---------- Address mode ----------
    // Check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else reset USI.
    case USI_SLAVE_CHECK_ADDRESS:
        if ((USIDR == 0) || (( USIDR>>1 ) == TWI_slaveAddress))
        {
            overflowState = ( USIDR & 0x01 ) ? USI_SLAVE_SEND_DATA : USI_SLAVE_REQUEST_DATA;
            SET_USI_TO_SEND_ACK();
        }
        else
        {
            SET_USI_TO_TWI_START_CONDITION_MODE();
        }
        break;

    // ----- Master write data mode ------
    // Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
    case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
        if ( USIDR ) // If NACK, the master does not want more data.
        {
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        // From here we just drop straight into USI_SLAVE_SEND_DATA if the master sent an ACK

    // Copy data from buffer to USIDR and set USI to shift byte. Next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
    case USI_SLAVE_SEND_DATA:
        // Get data from Buffer
        if ( TWI_TxHead != TWI_TxTail )
        {
            TWI_TxTail = ( TWI_TxTail + 1 ) & TWI_TX_BUFFER_MASK;
            USIDR = TWI_TxBuf[TWI_TxTail];
        }
        else // If the buffer is empty then:
        {
            SET_USI_TO_READ_ACK();
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        overflowState = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
        SET_USI_TO_SEND_DATA();
        break;

    // Set USI to sample reply from master. Next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
    case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
        overflowState = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
        SET_USI_TO_READ_ACK();
        break;

    // ----- Master read data mode ------
    // Set USI to sample data from master. Next USI_SLAVE_GET_DATA_AND_SEND_ACK.
    case USI_SLAVE_REQUEST_DATA:
        overflowState = USI_SLAVE_GET_DATA_AND_SEND_ACK;
        SET_USI_TO_READ_DATA();
        break;

    // Copy data from USIDR and send ACK. Next USI_SLAVE_REQUEST_DATA
    case USI_SLAVE_GET_DATA_AND_SEND_ACK:
        // Put data into Buffer
        TWI_RxHead = ( TWI_RxHead + 1 ) & TWI_RX_BUFFER_MASK;
        TWI_RxBuf[TWI_RxHead] = USIDR;

        overflowState = USI_SLAVE_REQUEST_DATA;
        SET_USI_TO_SEND_ACK();
        break;

    default:
        break;
    }
}
