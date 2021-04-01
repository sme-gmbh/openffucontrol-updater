#include "openffucontrolocuhandler.h"

OpenFFUcontrolOCUhandler::OpenFFUcontrolOCUhandler(QObject *parent, ModbusHandler *modbushandler)
{
    m_modbusHander = modbushandler;
}

int OpenFFUcontrolOCUhandler::sendRawCommand(quint16 slaveAddress, quint16 functonCode)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode;
    return m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode));
}

int OpenFFUcontrolOCUhandler::sendRawCommand(quint16 slaveAddress, quint16 functonCode, QByteArray payload)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode << payload.toHex();
    return m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode, payload));
}

int OpenFFUcontrolOCUhandler::auxEepromErase(qint16 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_AUX_EEPROM_ERASE);
    return m_modbusHander->sendRawRequest(request);
}

QByteArray OpenFFUcontrolOCUhandler::createRequest(quint16 slaveAddress, quint16 functionCode)
{
    QByteArray buffer;
    buffer.append(slaveAddress);
    buffer.append(functionCode);
    qDebug() << "openFFUcontrolOCUahndler: createRequest, request is: 0x" << buffer.toHex().data();
    return buffer;
}

QByteArray OpenFFUcontrolOCUhandler::createRequest(quint16 slaveAddress, quint16 functionCode, QByteArray payload)
{
    QByteArray buffer;
    buffer.append(slaveAddress);
    buffer.append(functionCode);
    buffer.append(payload);
    qDebug() << "openFFUcontrolOCUahndler: createRequest, request is: 0x" << buffer.toHex().data();
    return buffer;
}
