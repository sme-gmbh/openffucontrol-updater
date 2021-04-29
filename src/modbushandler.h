#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QDebug>
#include <QObject>

//#include <modbus/modbus-rtu.h>
#include "modbus.h"

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent, QString interface, bool isDryRun = false);
    explicit ModbusHandler(QObject *parent, QString interface, int baud, char parity, int data_bit, int stop_bit, bool isDryRun = false);

    bool open();
    void close();
    void setBaudRate(quint16 baudRate);
    void setParity(char parity);
    QByteArray sendRawRequest(QByteArray request);

signals:

private:
    QString m_interface;
    bool m_isDryRun = false;
    int m_baud = 9600;
    char m_parity = 'N';
    int m_data_bit = 8;
    int m_stop_bit = 1;

//    modbus_t *m_bus;

//    QModbusRtuSerialMaster *m_busmaster;

    ModBus *m_modbus;

public slots:

signals:

};

#endif // MODBUSHANDLER_H
