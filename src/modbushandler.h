#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QDebug>
#include <QObject>

#include <modbus/modbus-rtu.h>

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent, QString interface, bool isDryRun = false);
    explicit ModbusHandler(QObject *parent, QString interface, int baud, char parity, int data_bit, int stop_bit, bool isDryRun = false);

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
    bool m_isDryRun = false;
    int m_baud = 19200;
    char m_parity = 'E';
    int m_data_bit = 8;
    int m_stop_bit = 1;

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

};

#endif // MODBUSHANDLER_H
