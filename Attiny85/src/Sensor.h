#ifndef SENSOR_H_
#define SENSOR_H_

#include "avr/io.h"

#define LIMIT_CLOSED 28U	   // < 115 - замыкание
#define LIMIT_NAMUR_CLOSED 42U // < 170 - намур замкнут
#define LIMIT_NAMUR_OPEN 200U  // < 800 - намур разомкнут
							   // > - обрыв

#define TRIES 3 //Сколько раз после окончания импульса
				//должно его не быть, чтобы мы зарегистририровали импульс

/*

*/
enum SensorState
{
	SS_NONE,
	SS_CLOSE,
	SS_NAMUR_CLOSE,
	SS_NAMUR_OPEN,
	SS_OPEN
};

// Данные счетчика
struct DataCounter
{
	// Счетчик
	uint32_t counter;
	// Состояние
	uint8_t state;
	// АЦП
	uint8_t adc;
};

// Данные счетчика в EEPROM
struct Store
{
	// старшие 3 байта счетчика
	uint8_t Val[3];
	// буффер снижения ресурса
	uint8_t t[64];
};

// Структура данных Waterius-а
struct NewHeader
{
	//Версия прошивки
	uint8_t version;
	//Причина перезагрузки (регистр MCUSR datasheet 8.5.1):
	uint8_t service;
	//Количество перезагрузок.
	uint8_t resets;
	//Модификация 0 - Классический. 2 счетчика; 1 - 4C2W. 4 счетчика;
	uint8_t model;
	// Данные 1го счетчика (счетчик, состояние, ацп)
	DataCounter chanel1;
	// Данные 2го счетчика (счетчик, состояние, ацп)
	DataCounter chanel2;
	// HEADER_DATA_SIZE
	uint8_t crc;
	// Режим работы
	uint8_t mode;
	// Период передачи данных
	uint16_t wakeupPeriod;
} __attribute__((packed));

/*
	Класс подсчета импульсов.
	Управление подтяжкой, выбор канала АЦП производится перед вызовом check()
	Значенеи счетчика считывается из EEPROM по уразанному адресу.
	Новое значение автоматически сохраняется в EEPROM
	Значение записывается в память: старшие 3 байта в первые 3 байта адреса,
	4й младший байт записывается в одну из N ячеек в соответствии со
	значением 3го байта (от старшего к младшему)
	N=64; 100000 * 64 = 6 400 000 записей
	что 6400куб.метров при 1литре на импульс для каждого счетчика.
*/
class Sensor
{
public:
	explicit Sensor(DataCounter *_data, Store *address);
	// чтение значения из памяти
	void eeReadCounter();
	// Сохранение значения в память
	void eeWriteCounter();
	// Проверка состояния счетчика
	void check();

private:
	// Данные счетчика
	DataCounter *_data;
	uint8_t _timer;
	// Адрес хранения в EEPROM
	const Store *_address;
};
#endif
