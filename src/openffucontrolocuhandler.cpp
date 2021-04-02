#include "openffucontrolocuhandler.h"

OpenFFUcontrolOCUhandler::OpenFFUcontrolOCUhandler(QObject *parent, ModbusHandler *modbushandler)
{
    m_modbusHander = modbushandler;
}
// returns ocuExeptionCode 0 if only data was sent
quint8 OpenFFUcontrolOCUhandler::sendRawCommand(quint8 slaveAddress, quint16 functonCode)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode;
    m_response = parseOCUResponse( m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode)));
    return m_response.exeptionCode;
}
// returns ocuExeptionCode 0 if only data was sent
quint8 OpenFFUcontrolOCUhandler::sendRawCommand(quint8 slaveAddress, quint16 functonCode, QByteArray payload)
{
    qDebug() << "OpenFFUcontrolOCUhandler sendRawCommand" << slaveAddress << functonCode << payload.toHex();
    m_response = parseOCUResponse( m_modbusHander->sendRawRequest(createRequest(slaveAddress, functonCode, payload)));
    return m_response.exeptionCode;
}

bool OpenFFUcontrolOCUhandler::auxEepromErase(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_AUX_EEPROM_ERASE);
    ocuResponse response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

    if (response.exeptionCode == E_ACKNOWLEDGE){
        return true;
    }

    return false;
}

int OpenFFUcontrolOCUhandler::copyAuxEepromToFlash(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_COPY_EEPROM_TO_FLASH);
    ocuResponse response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

    if (response.exeptionCode == E_ACKNOWLEDGE){
        return true;
    }

    return false;
}
// returns true if system is busy
bool OpenFFUcontrolOCUhandler::systemBusy(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_STATUS_READ);
    ocuResponse response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

    if (response.exeptionCode == E_ACKNOWLEDGE){
        return false;
    }

    return true;
}
// Requests ocu application boot. OCU does not confirm the success.
void OpenFFUcontrolOCUhandler::bootApplication(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_AUX_EEPROM_ERASE);
    m_modbusHander->sendRawRequest(request);
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
