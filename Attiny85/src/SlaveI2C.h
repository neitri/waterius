#ifndef _SLAVEI2C_h
#define _SLAVEI2C_h

//#include <Arduino.h>
#include "Sensor.h"

#define I2C_SLAVE_ADDRESS ((unsigned char)10)

#define USI_RX_BUFFER_SIZE	8
#define USI_RX_BUFFER_MASK	(USI_RX_BUFFER_SIZE - 1)

#define USI_TX_BUFFER_SIZE	8
#define USI_TX_BUFFER_MASK	(USI_TX_BUFFER_SIZE - 1)

/* attiny2313 specific */
#define USI_DDR			DDRB
#define USI_PORT		PORTB
#define USI_PIN			PINB
#define USI_PORT_SDA	PORTB0
#define USI_PORT_SCL	PORTB2
#define USI_PIN_SDA		PINB0
#define USI_PIN_SCL		PINB2

/* states */
#define USI_SLAVE_CHECK_ADDR					0x00
#define USI_SLAVE_SEND_DATA						0x01
#define USI_SLAVE_REQ_REPLY_FROM_SEND_DATA		0x02
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA	0x03
#define USI_SLAVE_REQUEST_DATA					0x04
#define USI_SLAVE_GET_DATA_AND_ACK				0x05


#define NORMAL_MODE 0
#define SETUP_MODE 1
#define TRANSMIT_MODE 2



uint8_t crc_8(const void *data, const uint8_t num_bytes);

class SlaveI2C
{
 protected:
     //static uint8_t txBuffer[TX_BUFFER_SIZE];
     //static uint8_t txBufferPos;
     static uint8_t* bufferPointer;
     static uint8_t bufferSize;

     //static bool masterSentSleep;
     static bool _started;

     static void requestEvent();
     static void newCommand();
     static void receiveEvent( int howMany );
     static void getWakeUpPeriod();
 public:
     SlaveI2C();
     static void begin();
     static void end();
     //static bool masterGoingToSleep();
};



#endif

