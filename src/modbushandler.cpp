#include "modbushandler.h"
#include <QThread>

// uses default parameters if not set otherwise
ModbusHandler::ModbusHandler(QObject *parent, QString interface, bool isDryRun, bool debug)
{
    m_interface = interface;
    m_debug = debug;
    m_isDryRun = isDryRun;
}

ModbusHandler::ModbusHandler(QObject *parent, QString interface, int baud, char parity, int data_bit, int stop_bit, bool isDryRun, bool debug)
{
    m_interface = interface;
    m_debug = debug;
    m_isDryRun = isDryRun;
    m_baud = baud;
    m_parity = parity;
    m_data_bit = data_bit;
    m_stop_bit = stop_bit;
}

bool ModbusHandler::open()
{
    m_modbus = new ModBus(this, m_interface, m_debug);
    // TODO: Need to implement parity and stop bits
    // Caution with modbus: Always 2 stop bits in case of no parity, else 1
    if (m_modbus->open(m_baud))
    {
        fprintf(stdout, "ModbusHandler: Connected to device: %s\n", m_interface.toLocal8Bit().data());
    }
    else
    {
        fprintf(stdout, "ModbusHandler: Unable to open interface %s\n", m_interface.toLocal8Bit().data());
    }

    return true;
}

void ModbusHandler::close()
{
    m_modbus->close();
    delete m_modbus;
}

void ModbusHandler::setBaudRate(quint16 baudRate)
{
    m_baud = baudRate;
}

void ModbusHandler::setParity(char parity)
{
    m_parity = parity;
}

QByteArray ModbusHandler::sendRawRequest(QByteArray request)
{
    quint8 adr = request.at(0);
    quint8 fc = request.at(1);
    request.remove(0, 2);   // Strip function code

    if (!m_isDryRun)
    {
        return m_modbus->sendRawRequestBlocking(adr, fc, request);
    }
    else
        return QByteArray();
}
