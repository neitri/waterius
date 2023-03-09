#ifndef __WATERIUS_h
#define __WATERIUS_h

// model
#define WATERIUS_CLASSIC 0 // attiny85 - 2 счетчика импульсов
#define WATERIUS_4C2W 1

#define I2C_SLAVE_ADDRESS 10

enum CounterState_e
{
    CLOSE,
    NAMUR_CLOSE,
    NAMUR_OPEN,
    OPEN
};

enum class WateriusCommand{
    GETMODE='M',
    GETDATA='B',
    GOTOSLEEP='Z',
    TRANSMIT='T',
    SETWAKEUP='S'
};

enum class WateriusMode{
    SETUP=1,
    TRANSMIT=2,
    MANUAL_TRANSMIT=3
};

struct Data
{
    uint32_t value0;
    uint32_t value1;
};

struct CounterState
{                   // не добавляем в Data, т.к. та в буфере кольцевом
    uint8_t state0; // состояние входа
    uint8_t state1;
};

struct ADCLevel
{
    uint16_t adc0;
    uint16_t adc1;
};

#pragma pack(push, 1)
struct Header
{

    /*
    Версия прошивки
    */
    uint8_t version;

    /*
    Причина перезагрузки (регистр MCUSR datasheet 8.5.1):
         0001 - PORF: Power-on Reset Flag. Напряжение питания было низкое или 0.
         0010 - EXTRF: External Reset Flag. Пин ресет был в низком уровне.
         0100 - BORF: Brown-out Reset Flag. Напряжение питание было ниже требуемого.
         1000 - WDRF: Watchdog Reset Flag. Завершение работы таймера.

    8  - 1000 - WDRF
    9  - 1001 - WDRF + PORF
    10 - 1010 - WDRF + EXTRF
    */
    uint8_t service;

    /*
    ver 24: убрал напряжение
    */
    uint16_t reserved;

    /*
    Для совместимости с 0.10.0.
    */
    uint8_t reserved2;

    /*
    Включение режима настройки
    */
    uint8_t setup_started_counter;

    /*
    Количество перезагрузок
    */
    uint8_t resets;

    /*
    Модификация
    0 - Классический. 2 счетчика
    1 - 4C2W. 4 счетчика
    */
    uint8_t model;

    CounterState states; // TODO убрать
    Data data;
    ADCLevel adc;

    // HEADER_DATA_SIZE

    uint8_t crc;
}; // 23 байт
#pragma pack(pop)

extern uint8_t crc_8(uint8_t *b, size_t num_bytes, uint8_t crc = 0);

#endif