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
    void setBaudRate(quint16 baudRate);
    QByteArray sendRawRequest(QByteArray request);

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

signals:

};

#endif // MODBUSHANDLER_H
