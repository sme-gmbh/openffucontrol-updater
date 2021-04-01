#ifndef OPENFFUCONTROLOCUHANDLER_H
#define OPENFFUCONTROLOCUHANDLER_H

#include <QObject>

#include "modbushandler.h"

class OpenFFUcontrolOCUhandler : public QObject
{
    Q_OBJECT
public:
    explicit OpenFFUcontrolOCUhandler(QObject *parent, ModbusHandler* modbushandler);

    int sendRawCommand(quint16 slaveAddress, quint16 functonCode);
    int sendRawCommand(quint16 slaveAddress, quint16 functonCode, QByteArray payload);
    int auxEepromErase(qint16 slaveAddress);

    QByteArray createRequest(quint16 slaveAddress, quint16 functionCode);
    QByteArray createRequest(quint16 slaveAddress, quint16 functionCode, QByteArray payload);

    typedef enum {  // OCU function codes
        OCU_AUX_EEPROM_ERASE= 65,
        OCU_AUX_EEPROM_WRITE = 66,
        OCU_AUX_EEPROM_READ  = 67,
        OCU_INT_FLASH_ERASE  = 68,
        OCU_COPY_EEPROM_TO_FLASH = 69,
        OCU_INT_FLASH_READ = 70,
        OCU_STATUS_READ = 71,   // ACK if system is not busy
        OCU_BOOT_APPLICATION = 72,
        OCU_INT_EEPROM_READ = 100,
        OCU_INT_EEPROM_WRITE = 101
    } OCUfunctionCodes;

private:
    ModbusHandler* m_modbusHander;

signals:

};

#endif // OPENFFUCONTROLOCUHANDLER_H
