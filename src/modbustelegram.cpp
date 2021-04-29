#include "modbustelegram.h"

ModBusTelegram::ModBusTelegram()
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    this->m_id = id;
    id++;
    if (id == 0)            // Wrap around and avoid 0 - if that might ever happen with quint64... - just to be correct.
        id = 1;

    repeatCount = 1;
}

ModBusTelegram::ModBusTelegram(quint8 slaveAddress, quint8 functionCode, QByteArray data)
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    this->m_id = id;
    id++;
    if (id == 0)            // Wrap around and avoid 0 - if that might ever happen with quint64... - just to be correct.
        id = 1;

    this->slaveAddress = slaveAddress;
    this->functionCode = functionCode;
    this->data = data;
    repeatCount = 1;
}

bool ModBusTelegram::needsAnswer()
{
    if (this->slaveAddress == 0)
        return false;
    else
        return true;
}

quint64 ModBusTelegram::getID()
{
    return m_id;
}
