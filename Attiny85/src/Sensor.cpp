#include "Sensor.h"
#include <avr/eeprom.h>

//uint8_t Sensor::mask=0xff;

Sensor::Sensor(DataCounter* data, Store* address)
	: _data(data)
    , _address(address)
{
	_data->state=SS_NONE;
	eeReadCounter();
}

void Sensor::check(){	

    ADCSRA |= _BV(ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC))
    {
    };

    uint8_t value = ADCH;

    if (value<LIMIT_NAMUR_CLOSED){
        if(value<LIMIT_CLOSED){
            _data->state = SensorState::SS_CLOSE;
        }else{
            _data->state = SensorState::SS_NAMUR_CLOSE;
        }
        if (_timer >= 0)
        {
            _timer--;
        }
        if (_timer == 0)
        {
            _data->counter++;
			eeWriteCounter();
        }
    }else if(value>LIMIT_NAMUR_OPEN){
        _data->state = SensorState::SS_NAMUR_OPEN;
        _timer = TRIES;
    }
}

void Sensor::eeReadCounter(){
	uint8_t *ptr =(uint8_t*)&_data->counter+3;
	uint8_t* address=(uint8_t*)_address;
    uint8_t mask=0xff;
    for(uint8_t i=3;i;i--){
        *ptr = eeprom_read_byte((uint8_t*)address);
        mask&=EEDR;
        ptr--;address++;
    }
    address=(uint8_t*)_address + ((*(uint8_t*)(&_data->counter+2)) & 0x3f)+3;
    *ptr = eeprom_read_byte((uint8_t*)address);
    mask&=EEDR;
    if(mask){
        _data->counter=0;
    }
}

void Sensor::eeWriteCounter(){
	uint8_t *ptr =(uint8_t*)&_data->counter+3;
	uint8_t* address=(uint8_t*)_address;
    eeprom_update_byte((uint8_t*)address,*ptr);
    ptr++;
    address=(uint8_t*)_address+3;
    for(uint8_t i=3;i;i--){
        eeprom_update_byte((uint8_t*)address,*ptr);
        ptr++;address--;
    }
}
