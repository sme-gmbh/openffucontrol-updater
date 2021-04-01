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
    int sendRawRequest(QByteArray request);

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
