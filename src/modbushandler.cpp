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
//    if (m_isDryRun == true){
//        fprintf(stderr, "ModbusHandler::open(): DRY RUN: Modbus interface configured and connected.\n");
//        return true;
//    }
//    QByteArray interface_ba = m_interface.toLocal8Bit();
//    m_bus = modbus_new_rtu(interface_ba.data(), m_baud, m_parity, m_data_bit, m_stop_bit);

//    m_busmaster = new QModbusRtuSerialMaster();
//    m_busmaster->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_interface);
//    m_busmaster->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
//    m_busmaster->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baud);
//    m_busmaster->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
//    m_busmaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
    //m_busmaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
//    if (m_busmaster->connectDevice())
//    {
//        qDebug() << "Connected, state is: " << m_busmaster->state();
//        return true;
//    }
//    else
//    {
//        qDebug() << "Connection failed, error is: " << m_busmaster->errorString();
//        return false;
//    }

    m_modbus = new ModBus(this, m_interface);
    if (m_modbus->open(m_baud))
    {
        fprintf(stdout, "ModbusHandler: Connected to device: %s\n", m_interface.toLocal8Bit().data());
    }
    else
    {
        fprintf(stdout, "ModbusHandler: Unable to open interface %s\n", m_interface.toLocal8Bit().data());
    }

//    if (m_bus == nullptr) {
//        fprintf(stdout, "ModbusHandler: Unable to open interface %s\n", m_interface.toLocal8Bit().data());
//        return false;
//    }

//    if (modbus_connect(m_bus) == -1) {
//        fprintf(stdout, "ModbusHandler: Unable to connect to device: %s\n", QString(modbus_strerror(errno)).toLocal8Bit().data());
//        modbus_free(m_bus);
//        return false;
//    }

//    fprintf(stderr, "ModbusHandler::open(): Modbus interface configured and connected.\n");
    return true;
}

void ModbusHandler::close()
{
//    if (m_isDryRun == true){
//        fprintf(stderr, "ModbusHandler::close(): DRY RUN: Modbus interface closed.\n");
//        return;
//    }

//    modbus_close(m_bus);
//    modbus_free(m_bus);

//    m_busmaster->disconnectDevice();
//    delete m_busmaster;
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
// returns empty QByteArray if occurs, else the raw response to the request
QByteArray ModbusHandler::sendRawRequest(QByteArray request)
{
//    quint8 rawResponse[MODBUS_RTU_MAX_ADU_LENGTH];
//    int requestLength = -1;

//    if (m_isDryRun == true){
//        fprintf(stdout, "ModbusHandler::sendRawRequest(): DRY RUN: 0x%s.\n", request.toHex().data());
//        return nullptr;
//    }

    quint8 adr = request.at(0);
    quint8 fc = request.at(1);
    request.remove(0, 2);   // Strip function code
//    QModbusPdu pdu;
//    pdu.setFunctionCode(static_cast<QModbusPdu::FunctionCode>(fc));
//    pdu.setData(request);
//    QModbusRequest r(pdu);
//    QModbusReply* rep = m_busmaster->sendRawRequest(r, m_adr);

//    while(!rep->isFinished())
//    {
//        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
//        QThread::msleep(50);
//    }

//    QModbusResponse res = rep->rawResult();

//    return res.data();

//    requestLength = modbus_send_raw_request(m_bus, (unsigned char*)request.constData(), request.length());
//    if (requestLength == -1){
//        fprintf(stderr, "ModbusHandler::sendRawRequest(): Unable to send request. Libmodbus error: %s\n", modbus_strerror(errno));
//        return QByteArray();
//    }

//    requestLength = modbus_receive_confirmation(m_bus, rawResponse);
//    if (requestLength == -1){
//        fprintf(stderr, "ModbusHandler::sendRawRequest(): No response to sent request. Libmodbus error: %s\n", modbus_strerror(errno));
//        return QByteArray();
//    }
//    QByteArray response((char*)rawResponse, requestLength);
//    return response;

    QByteArray response = m_modbus->sendRawRequestBlocking(adr, fc, request);

    return response;
}
