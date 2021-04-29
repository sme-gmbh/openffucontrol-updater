#ifndef MODBUSTELEGRAM_H
#define MODBUSTELEGRAM_H

#include <QByteArray>

class ModBusTelegram
{
public:
    ModBusTelegram();
    ModBusTelegram(quint8 slaveAddress, quint8 functionCode, QByteArray data);

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
