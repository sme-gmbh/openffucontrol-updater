#include "modbushandler.h"
#include <QThread>

// uses default parameters if not set otherwise
ModbusHandler::ModbusHandler(QObject *parent, QString interface, bool isDryRun)
{
    m_interface = interface;
    m_isDryRun = isDryRun;
}

ModbusHandler::ModbusHandler(QObject *parent, QString interface, int baud, char parity, int data_bit, int stop_bit, bool isDryRun)
{
    m_interface = interface;
    m_isDryRun = isDryRun;
    m_baud = baud;
    m_parity = parity;
    m_data_bit = data_bit;
    m_stop_bit = stop_bit;
}

bool ModbusHandler::open()
{
    if (m_isDryRun == true){
        fprintf(stderr, "ModbusHandler::open(): DRY RUN: Modbus interface configured and connected.\n");
        return true;
    }
    QByteArray interface_ba = m_interface.toLocal8Bit();
    m_bus = modbus_new_rtu(interface_ba.data(), m_baud, m_parity, m_data_bit, m_stop_bit);
    if (m_bus == nullptr) {
        fprintf(stdout, "ModbusHandler: Unable to open interface %s\n", m_interface.toLocal8Bit().data());
        return false;
    }

    if (modbus_connect(m_bus) == -1) {
        fprintf(stdout, "ModbusHandler: Unable to connect to device: %s\n", QString(modbus_strerror(errno)).toLocal8Bit().data());
        modbus_free(m_bus);
        return false;
    }

    fprintf(stderr, "ModbusHandler::open(): Modbus interface configured and connected.\n");
    return true;
}

void ModbusHandler::close()
{
    if (m_isDryRun == true){
        fprintf(stderr, "ModbusHandler::close(): DRY RUN: Modbus interface closed.\n");
        return;
    }

    modbus_close(m_bus);
    modbus_free(m_bus);
}

void ModbusHandler::setSlaveAddress(quint16 adr)
{
    if (m_isDryRun == true){
        fprintf(stdout, "ModbusHandler::setSlaveAddress(): DRY RUN: Slave addres set.\n");
        return;
    }
    modbus_set_slave(m_bus, adr);
}

void ModbusHandler::setBaudRate(quint16 baudRate)
{
    m_baud = baudRate;
}

void ModbusHandler::setParity(char parity)
{
    m_parity = parity;
}
// returns empty QByteArray if occurs, else the raw response to the request
QByteArray ModbusHandler::sendRawRequest(QByteArray request)
{
    QByteArray response;
    quint8 rawResponse[MODBUS_RTU_MAX_ADU_LENGTH];
    int requestLength = -1;

    if (m_isDryRun == true){
        fprintf(stdout, "ModbusHandler::sendRawRequest(): DRY RUN: 0x%s.\n", request.toHex().data());
        return nullptr;
    }

    requestLength = modbus_send_raw_request(m_bus, (unsigned char*)request.constData(), request.length());
    if (requestLength == -1){
        fprintf(stderr, "ModbusHandler::sendRawRequest(): Unable to send request. Libmodbus error: %s\n", modbus_strerror(errno));
        return response;
    }

    requestLength = modbus_receive_confirmation(m_bus, rawResponse);
    if (requestLength == -1){
        fprintf(stderr, "ModbusHandler::sendRawRequest(): No response to sent request. Libmodbus error: %s\n", modbus_strerror(errno));
        return response;
    }
    response = QByteArray::fromRawData((char*)rawResponse, requestLength);
    return response;
}
