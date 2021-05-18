#ifndef OPENFFUCONTROLOCUHANDLER_H
#define OPENFFUCONTROLOCUHANDLER_H

#include <QObject>
#include <QThread>

#include "modbus.h"

class OpenFFUcontrolOCUhandler : public QObject
{
    Q_OBJECT
public:
    explicit OpenFFUcontrolOCUhandler(QObject *parent, ModBus* modbus, bool dryRun = false, bool debug = false);

    quint8 sendRawCommand(quint8 slaveAddress, quint16 functonCode, QByteArray payload);

    // OCU Commands

    // auxiliary EEPROM options
    bool auxEepromErase(quint8 slaveAddress);
    int auxEepromWrite(quint8 slaveAddress, quint32 writeStartAddress, QByteArray data);
    QByteArray auxEepromRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount);

    // internal flash options
    int intFlashErase(quint8 slaveAddress);
    int copyAuxEepromToFlash(quint8 slaveAddress);
    QByteArray intFlashRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount);

    // internal EEPROM options
    int intEepromWrite(quint8 slaveAddress, quint32 writeStartAddress, QByteArray data);
    QByteArray intEepromRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount);

    // system properties
    bool systemBusy(quint8 slaveAddress);
    void bootApplication(quint8 slaveAddress);

    QString errorString(quint8 errorCode);

    QByteArray getResponsePayload();
    bool updateFirmware(quint8 slaveAddress, QByteArray application);

private:

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

    typedef enum {
        E_PARSER_FAILED = 0xff
    } ParserExeptions;

    typedef enum {
        E_UNKNOWN_ERROR = 0xfe,
        E_NO_ERROR = 0x00,
    } GenericErrors;

    struct ocuResponse{
        quint8 slaveId = 0;
        quint8 functionCode = 0;
        QByteArray payload;
        quint8 exeptionCode = 0;
        quint16 crc = 0;
    };

    bool isDryRun = false;
    bool isDebug = false;
    ModBus* m_modbus;
    ocuResponse m_response;

    ocuResponse parseOCUResponse(QByteArray response);
    QByteArray assembleAddressHeader(quint64 startAddress, quint16 byteCount);
    void waitForOCU(quint8 slaveAddress);

signals:

};

#endif // OPENFFUCONTROLOCUHANDLER_H
