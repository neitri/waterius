#include "Storage.h"
#include <Waterius.h>

template <class T>
EEPROMStorage<T>::EEPROMStorage(const uint8_t _blocks, const uint8_t _start_addr)
    : start_addr(_start_addr), activeBlock(0), blocks(_blocks)
{
    elementSize = sizeof(T);
    flag_shift = start_addr + elementSize * blocks;

    //тут определяем не испорчена ли память
    int t = 0;
    T tmp;
    for (int i = 0; i < blocks; i++)
    {
        t += EEPROM.read(flag_shift + i);
        if (t > 0)
        {
            activeBlock = i;
            if (get(tmp))
                return;
        }
    }

    activeBlock = 0;
    for (uint16_t i = start_addr; i < flag_shift + blocks; i++)
    {
        EEPROM.write(i, 0);
    }
}

template <class T>
void EEPROMStorage<T>::add(const T &element)
{
    uint8_t prev = activeBlock;
    activeBlock = (activeBlock < blocks - 1) ? activeBlock + 1 : 0;

    EEPROM.put(start_addr + activeBlock * elementSize, element);
    uint8_t mark = crc_8((unsigned char *)&element, elementSize);
    if (mark == 0)
        mark++;
    EEPROM.write(flag_shift + activeBlock, mark);
    EEPROM.write(flag_shift + prev, 0);
}

template <class T>
bool EEPROMStorage<T>::get(T &element)
{
    return get_block(activeBlock, element);
}

template <class T>
bool EEPROMStorage<T>::get_block(const uint8_t block, T &element)
{
    T tmp;
    EEPROM.get(start_addr + block * elementSize, tmp);

    uint8_t crc = crc_8((unsigned char *)&tmp, elementSize);
    uint8_t mark = EEPROM.read(flag_shift + block);
    if (mark == crc || (mark == 1 && crc == 0))
    {
        element = tmp;
        return true;
    }
    return false;
}

template <class T>
uint16_t EEPROMStorage<T>::size()
{
    return flag_shift + blocks;
}

template class EEPROMStorage<Data>;
