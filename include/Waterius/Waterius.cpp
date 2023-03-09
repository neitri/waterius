#include <stdint.h>
#include <stddef.h>
#include "Waterius.h"

/**
 * @brief Расчет контрольной суммы Dallas CRC x8+x5+x4+1
 * 
 * @link https://github.com/lammertb/libcrc/blob/600316a01924fd5bb9d47d535db5b8f3987db178/src/crc8.c
 * @link https://gist.github.com/brimston3/83cdeda8f7d2cf55717b83f0d32f9b5e
 * @link https://www.onlinegdb.com/online_c++_compiler
 * 
 * @param b указатель на данные для подсчета контрольной суммы
 * @param num_bytes количество байт для подсчета
 * @param crc контрольная сумма предыдущих данных
 * @return uint8_t подсчитанная контрольная сумма
 */
uint8_t crc_8(uint8_t *b, size_t num_bytes, uint8_t crc)
{
    while (num_bytes--)
    {
        uint8_t inbyte = *b++;
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}
