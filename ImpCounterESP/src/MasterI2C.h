#ifndef _MASTERI2C_h
#define _MASTERI2C_h

#include <Arduino.h>

#define I2C_SLAVE_ADDR 10


struct SlaveStats {
	uint32_t bytesReady;
	uint32_t masterWakeEvery;
	uint32_t measurementEvery;
	uint32_t vcc;
	uint8_t bytesPerMeasurement;
	uint8_t version;
	uint8_t numberOfSensors;
	uint8_t dummy;
	uint32_t deviceID;
}; //should be *16bit https://github.com/esp8266/Arduino/issues/1825


class MasterI2C
{
 protected:
	 void gotoFirstByte();
	 byte getNextByte();
	 uint32_t getUint();
	 uint8_t getByte();

 public:
	 void begin();
	 void sendCmd( const char cmd );
	 uint16_t getSlaveStorage( byte* storage, const uint16_t maxStorageSize, const uint32_t bytesToFetch );
	 SlaveStats getSlaveStats();
};


#endif

