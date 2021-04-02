#include "openffucontrolocuhandler.h"

OpenFFUcontrolOCUhandler::OpenFFUcontrolOCUhandler(QObject *parent, ModbusHandler *modbushandler)
{
    m_modbusHander = modbushandler;
}

QByteArray OpenFFUcontrolOCUhandler::sendRawCommand(quint8 slaveAddress, quint16 functonCode)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode;
    return m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode));
}

QByteArray OpenFFUcontrolOCUhandler::sendRawCommand(quint8 slaveAddress, quint16 functonCode, QByteArray payload)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode << payload.toHex();
    return m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode, payload));
}

QByteArray OpenFFUcontrolOCUhandler::auxEepromErase(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_AUX_EEPROM_ERASE);
    return m_modbusHander->sendRawRequest(request);
}

QByteArray OpenFFUcontrolOCUhandler::createRequest(quint8 slaveAddress, quint8 functionCode)
{
    QByteArray buffer;
    buffer.append(slaveAddress);
    buffer.append(functionCode);
    qDebug() << "openFFUcontrolOCUahndler: createRequest, request is: 0x" << buffer.toHex().data();
    return buffer;
}

QByteArray OpenFFUcontrolOCUhandler::createRequest(quint8 slaveAddress, quint8 functionCode, QByteArray payload)
{
    QByteArray buffer;
    buffer.append(slaveAddress);
    buffer.append(functionCode);
    buffer.append(payload);
    qDebug() << "openFFUcontrolOCUahndler: createRequest, request is: 0x" << buffer.toHex().data();
    return buffer;
}

OpenFFUcontrolOCUhandler::ocuResponse OpenFFUcontrolOCUhandler::parseOCUResponse(QByteArray response)
{
    ocuResponse parsed;

    parsed.slaveId = response.at(0);
    parsed.functionCode = response.at(1);
    parsed.payload = response.right(response.length() - 2);

    // exeption responses use reqest_functionCode + 0x80 as indication for errors
    if (parsed.functionCode >= 0x80){
        // exeption codes are only one byte, if there are more ther must be an issue in parsing
        if (parsed.payload.length() != 1){
            fprintf(stderr, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Failed to parse resonse. Response thought to be an OCU exeption but payload lengt is not 1");
            parsed.exeptionCode = E_PARSER_FAILED;
        } else {
            parsed.exeptionCode = response.at(2);
        }
    }

    return parsed;
}
