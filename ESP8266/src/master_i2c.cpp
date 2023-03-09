#include "master_i2c.h"
#include "Logging.h"
#include <Wire.h>
#include <Arduino.h>
#include "setup.h"
#include <Waterius.h>


void MasterI2C::begin()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000L);
    Wire.setClockStretchLimit(2500L); // Иначе связь с Attiny не надежная будут FF FF в хвосте посылки
}

bool MasterI2C::sendCmd(uint8_t cmd)
{
    return sendData(&cmd, 1);
}

bool MasterI2C::sendData(uint8_t *buf, size_t size)
{
    uint8_t i;
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    for (i = 0; i < size; i++)
    {
        if (Wire.write(buf[i]) != 1)
        {
            LOG_ERROR(F("I2C transmitting fail."));
            return false;
        }
    }
    int err = Wire.endTransmission(true);
    if (err != 0)
    {
        LOG_ERROR(F("end error:") << err);
        return false;
    }

    return true;
}

bool MasterI2C::getByte(uint8_t &value, uint8_t &crc)
{

    if (Wire.requestFrom(I2C_SLAVE_ADDRESS, 1) != 1)
    {
        LOG_ERROR(F("RequestFrom failed"));
        return false;
    }
    value = Wire.read();
    crc = crc_8(&value, 1, crc);
    return true;
}

/**
 * @brief Чтение одного байта по указателю
 * 
 * @param value указатель приемника
 * @param crc контрольная сумма
 * @return true прочитано успешно
 * @return false данные не прочитаны
 */
bool MasterI2C::getByte(uint8_t *value, uint8_t &crc)
{
    if (Wire.requestFrom(I2C_SLAVE_ADDRESS, 1) != 1)
    {
        LOG_ERROR(F("RequestFrom failed"));
        return false;
    }
    *value = (uint8_t)Wire.read();
    crc = crc_8(value, 1, crc);
    return true;
}

/**
 * @brief Чтение заданного количества байт в буффер по указателю
 * 
 * @param value указатель на буфер приемника
 * @param count количество байт для чтения
 * @param crc контрольная сумма для расчета
 * @return true прочитано успешно
 * @return false данные не прочитаны
 */
bool MasterI2C::getBytes(uint8_t *value, uint8_t count, uint8_t &crc)
{
    for (; count > 0; count--, value++)
    {
        if (!getByte(value, crc))
            return false;
    }
    return true;
}

bool MasterI2C::getUint16(uint16_t &value, uint8_t &crc)
{

    uint8_t i1, i2;
    if (getByte(i1, crc) && getByte(i2, crc))
    {
        value = i2;
        value = value << 8;
        value |= i1;
        return true;
    }
    return false;
}

bool MasterI2C::getUint(uint32_t &value, uint8_t &crc)
{

    uint8_t i1, i2, i3, i4;
    if (getByte(i1, crc) && getByte(i2, crc) && getByte(i3, crc) && getByte(i4, crc))
    {
        value = i4;
        value = value << 8;
        value |= i3;
        value = value << 8;
        value |= i2;
        value = value << 8;
        value |= i1;
        //вот так не работает из-за преобразования типов:
        // value = i1 | (i2 << 8) | (i3 << 16) | (i4 << 24);
        return true;
    }
    return false;
}

bool MasterI2C::getMode(uint8_t &mode)
{

    uint8_t crc; // not used
    if (!sendCmd((uint8_t)WateriusCommand::GETMODE) || !getByte(&mode, crc))
    {
        LOG_ERROR(F("GetMode failed. Check i2c line."));
        return false;
    }
    LOG_INFO("mode: " << mode);
    return true;
}

/**
 * @brief Чтение данных с прибора.
 * 
 * Данные читаются в буффер, до контрольной суммы включительно. 
 * При успешном чтении расчитанная контрольная сумма должна быть равно 0, 
 * в противном случае во время чтения произошла ошибка.
 * 
 * @param data Структура для заполнения данными
 * @return true прочитанно успешно.
 * @return false произошла ошибка, код ошибки в data.diagnostic
 */
bool MasterI2C::getSlaveData(SlaveData &data)
{
    sendCmd((uint8_t)WateriusCommand::GETDATA);
    delay(1); // не успевает подсчитать crc
    data.diagnostic = WATERIUS_NO_LINK;
    Header buffer;
    uint8_t crc = 0;
    if (!getBytes((uint8_t *)&buffer, sizeof(Header), crc))
    {
        LOG_ERROR(F("Data failed"));
        return false;
    }
    data.version = buffer.version;
    data.service = buffer.service;
    data.setup_started_counter = buffer.setup_started_counter;
    data.resets = buffer.resets;
    data.model = buffer.model;
    data.state0 = buffer.states.state0;
    data.state1 = buffer.states.state1;

    data.impulses0 = buffer.data.value0;
    data.impulses1 = buffer.data.value1;
    data.adc0 = buffer.adc.adc0;
    data.adc1 = buffer.adc.adc1;
    data.crc = buffer.crc;
    
    LOG_INFO(F("version: ") << data.version);
    LOG_INFO(F("service: ") << data.service);
    LOG_INFO(F("setup_started_counter: ") << data.setup_started_counter);
    LOG_INFO(F("resets: ") << data.resets);
    LOG_INFO(F("MODEL: ") << data.model);
    LOG_INFO(F("state0: ") << data.state0);
    LOG_INFO(F("state1: ") << data.state1);
    LOG_INFO(F("impulses0: ") << data.impulses0);
    LOG_INFO(F("impulses1: ") << data.impulses1);
    LOG_INFO(F("adc0: ") << data.adc0);
    LOG_INFO(F("adc1: ") << data.adc1);
    
    if (crc)
    {
        LOG_ERROR(F("CRC wrong ")<<crc);
        data.diagnostic = WATERIUS_BAD_CRC;
        return false;
    }
    LOG_INFO(F("CRC ok"));
    data.diagnostic = WATERIUS_OK;
    return true;
}

bool MasterI2C::setWakeUpPeriod(uint16_t period)
{
    uint8_t txBuf[4];

    txBuf[0] = (uint8_t)WateriusCommand::SETWAKEUP;
    txBuf[1] = (uint8_t)(period >> 8);
    txBuf[2] = (uint8_t)(period);
    txBuf[3] = crc_8(&txBuf[1], 2, 0);

    if (!sendData(txBuf, 4))
    {
        return false;
    }
    return true;
}