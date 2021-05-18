#ifndef MODBUSTELEGRAM_H
#define MODBUSTELEGRAM_H

#include <QByteArray>

class ModBusTelegram
{
public:
    ModBusTelegram();
    ModBusTelegram(quint8 slaveAddress, quint8 functionCode, QByteArray data);

    typedef enum {
        E_ILLEGAL_FUNCTION = 0x01,
        E_ILLEGAL_DATA_ADDRESS = 0x02,
        E_ILLEGAL_DATA_VALUE = 0x03,
        E_SERVER_DEVICE_FAILURE = 0x04,
        E_ACKNOWLEDGE = 0x05,
        E_SERVER_DEVICE_BUSY = 0x06,
        E_MEMORY_PARITY_ERROR = 0x08,
        E_GATEWAY_PATH_UNAVAILABLE = 0x0a,
        E_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0x0b
    } ExeptionCode;

    quint8 slaveAddress;
    quint8 functionCode;
    QByteArray data;

    int repeatCount;    // Set to different value if that telegram is important and should be autorepeated

    bool needsAnswer();

    quint64 getID();

private:
    quint64 m_id; // Telegram id is unique accross all telegrams per bus
};

#endif // MODBUSTELEGRAM_H
