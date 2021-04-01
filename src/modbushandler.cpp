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
        fprintf(stderr, "ModbusHandler::setSlaveAddress(): DRY RUN: Slave addres set.\n");
        return;
    }
    modbus_set_slave(m_bus, adr);
}

//void ModbusHandler::slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg, quint16 rawdata)
//{
//    int result;
//    modbus_set_slave(m_bus, adr);
//    // Bus clearance time
//    QThread::msleep(100);
//    result = modbus_write_register(m_bus, reg, rawdata);
//    if (result >= 0)
//        emit signal_wroteHoldingRegisterData(telegramID);
//    else
//    {
//        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_write_register returned: ") + QString(modbus_strerror(errno) +
//                                             QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
//        emit signal_transactionLost(telegramID);
//    }
//}

//void ModbusHandler::slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg)
//{
//    int result;
//    uint16_t rawdata;
//    modbus_set_slave(m_bus, adr);
//    // Bus clearance time
//    QThread::msleep(100);
//    result = modbus_read_registers(m_bus, reg, 1, &rawdata);
//    if (result >= 0)
//        emit signal_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
//    else
//    {
//        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_registers returned: ") + QString(modbus_strerror(errno) +
//                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
//        emit signal_transactionLost(telegramID);
//    }
//}

//void ModbusHandler::slot_readInputRegisterData(quint64 telegramID, quint16 adr, ModbusHandler::ModbusHandler reg)
//{
//    int result;
//    uint16_t rawdata;
//    modbus_set_slave(m_bus, adr);
//    // Bus clearance time
//    QThread::msleep(100);
//    result = modbus_read_input_registers(m_bus, reg, 1, &rawdata);
//    if (result >= 0)
//        emit signal_receivedInputRegisterData(telegramID, adr, reg, rawdata);
//    else
//    {
//        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_input_registers returned: ") + QString(modbus_strerror(errno) +
//                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
//        emit signal_transactionLost(telegramID);
//    }
//}
