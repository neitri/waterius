
#include <Arduino.h>
#include <Wire.h>
#include "Setup.h"
#include "SlaveI2C.h"
#include "Sensor.h"
#include "Power.h"

extern struct NewHeader head;

#define CRC 3

#if CRC==1
// https://gist.github.com/brimston3/83cdeda8f7d2cf55717b83f0d32f9b5e
// https://www.onlinegdb.com/online_c++_compiler
// Dallas CRC x8+x5+x4+1
uint8_t crc8(const void *buffer, const uint8_t size) { //64
    uint8_t crc = 0;
    uint8_t* b=(uint8_t*)buffer;
    for (uint8_t a = size; a ; a--,b++) {
        uint8_t i = (*b ^ crc) & 0xff;
        //i = (*b ^ crc);
        crc = 0;
        if (i & 1)
            crc ^= 0x5e;
        if (i & 2)
            crc ^= 0xbc;
        if (i & 4)
            crc ^= 0x61;
        if (i & 8)
            crc ^= 0xc2;
        if (i & 0x10)
            crc ^= 0x9d;
        if (i & 0x20)
            crc ^= 0x23;
        if (i & 0x40)
            crc ^= 0x46;
        if (i & 0x80)
            crc ^= 0x8c;
    }
    return crc;
}
#elif CRC==2
byte crc__8(const void *buffer, const uint8_t size) {     //40
  byte crc = 0;
  uint8_t* b=(uint8_t*)buffer;
  for (byte i = size; i ; i--,b++) {
    uint8_t data = *b;
    for (uint8_t j = 8; j ; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}
#elif CRC==3
uint8_t crc_8(const void *buffer, const uint8_t size) {      // 30
  byte crc = 0;
  uint8_t* b=(uint8_t*)buffer;
  for (byte i = size; i ; i--,b++) {
    byte data = *(b);
    uint8_t counter;
    uint8_t temp;
    asm volatile (
      "EOR %[crc_out], %[data_in] \n\t"
      "LDI %[counter], 8          \n\t"
      "LDI %[temp], 0x8C          \n\t"
      "_loop_start_%=:            \n\t"
      "LSR %[crc_out]             \n\t"
      "BRCC _loop_end_%=          \n\t"
      "EOR %[crc_out], %[temp]    \n\t"
      "_loop_end_%=:              \n\t"
      "DEC %[counter]             \n\t"
      "BRNE _loop_start_%="
      : [crc_out]"=r" (crc), [counter]"=d" (counter), [temp]"=d" (temp)
      : [crc_in]"0" (crc), [data_in]"r" (data)
    );
  }
  return crc;
}
#endif

uint8_t* SlaveI2C::bufferPointer;
uint8_t SlaveI2C::bufferSize=0;

SlaveI2C::SlaveI2C()
{
bufferPointer=(uint8_t*)&head;
}


void SlaveI2C::begin() {
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onReceive( receiveEvent );
    Wire.onRequest( requestEvent );
    newCommand();
}

void SlaveI2C::end() {
    Wire.end();
}


void SlaveI2C::requestEvent() {
    if(bufferSize){
        bufferSize--;
    }
    Wire.write(*bufferPointer);
    if(bufferSize){
        bufferPointer++;
    }
}

void SlaveI2C::newCommand() {
    bufferPointer=(uint8_t*)&head;
    bufferSize=sizeof(NewHeader);
}

void SlaveI2C::receiveEvent(int howMany) {
    static uint8_t command;

    command = Wire.read();

    newCommand();
    switch (command) {
        case 'B':  // данные
            head.crc = crc_8(&head, HEADER_DATA_SIZE);
            bufferPointer=(uint8_t*)&head;
            bufferSize=TX_BUFFER_SIZE;
            break;
        case 'Z':  // Готовы ко сну
            ESPPowerPin::setModeNormal();
            break;
        case 'M':  // Разбудили ESP для настройки или передачи данных?
            bufferPointer=(uint8_t*)&head.mode;
            bufferSize=sizeof(uint8_t);
            break;
        case 'T':  // Не используется. После настройки ESP перезагрузим, поэтому меняем режим на передачу данных
            head.mode = TRANSMIT_MODE;
            break;
        case 'S': //ESP присылает новое значение периода пробуждения
            getWakeUpPeriod();
            break;

    }
}

void SlaveI2C::getWakeUpPeriod(){
    uint8_t data[3];

    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    //crc = Wire.read();

    //uint16_t newPeriod;
    //newPeriod = (data[0]<<8) | data[1];
    //uint16_t newPeriod = *(uint16_t*)&(data[0]);//(data[0]<<8) | data[1];

    //if ((crc == crc_8(data, 2)) && (newPeriod != 0)) { 
    if ((crc_8(&data, 3)==0) && (data[2] != 0)) {
        head.wakeupPeriod = *(uint16_t*)&(data[0]);
    }
}
