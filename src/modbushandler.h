#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QObject>

#include "loghandler.h"
#include <modbus/modbus-rtu.h>

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent, QString interface);

    bool open();
    void close();
    void setSlaveAddress(quint16 adr);

    typedef enum {
        OCU_AUX_EEPROM_ERASE= 65,
        OCU_AUX_EEPROM_WRITE = 66,
        OCU_AUX_EEPROM_READ  = 67,
        OCU_INT_FLASH_ERASE  = 68,
        OCU_COPY_EEPROM_TO_FLASH = 69,
        OCU_INT_FLASH_READ = 70,
        OCU_STATUS_READ = 71,   // ACK if system is not busy
        OCU_BOOT_APPLICATION = 72,
        OCU_INT_EEPROM_READ = 100,
        OCU_INT_EEPROM_WRITE = 101,

    } OCUfunctionCodes;

signals:

private:
    QString m_interface;
    modbus_t *m_bus;

public slots:
    //void slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg, quint16 rawdata);
    //void slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg);
    //void slot_readInputRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg);

signals:
    // Modbus result signals
    void signal_transactionLost(quint64 telegramID);
    //void signal_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg, quint16 rawdata);
    //void signal_receivedInputRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg, quint16 rawdata);
    void signal_wroteHoldingRegisterData(quint64 telegramID);

    // Log output signals
    void signal_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
    void signal_entryGone(LogEntry::LoggingCategory loggingCategory, QString module, QString text);

};

#endif // MODBUSHANDLER_H
