#ifndef _WATERIUS_SETUP_h
#define _WATERIUS_SETUP_h

#include <Arduino.h>

/*
    Включение логирования
    3 pin TX -> RX (TTL-USB 3.3 или 5в), 9600 8N1
    При логировании не работает счетчик2 на 3-м пине (Вход 2).

    #define LOG_ON
*/

#ifndef LOG_ON
#define LOG_BEGIN(x)
#define LOG(x)
#else
#undef LOG_BEGIN
#undef LOG

// TinyDebugSerial на PB3 только в attiny85, 1MHz
#include "TinyDebugSerial.h"
#define LOG_BEGIN(x) mySerial.begin(x)
#define LOG(x)                \
    mySerial.print(millis()); \
    mySerial.print(F(" : ")); \
    mySerial.println(x);
#endif

/*
   1 минута примерно равна 240 пробуждениям
*/
#define ONE_MINUTE 240L

/*
    Период отправки данных на сервер, мин.
*/
#define WAKEUP_PERIOD_DEFAULT 15L * ONE_MINUTE


/*
    Аварийное отключение, если ESP зависнет и не пришлет команду "сон".
*/
#define WAIT_ESP_MSEC 120000UL

/*
    Сколько милисекунд пользователь может
    настраивать ESP. Если не закончил, питание ESP выключится.
*/
#define SETUP_TIME_MSEC 600000UL

/*
    время долгого нажатия кнопки, милисекунд
*/
#define LONG_PRESS_MSEC 3000

/*
    время перехода в сон
*/
#define DELAY_SENT_SLEEP 2000

#endif