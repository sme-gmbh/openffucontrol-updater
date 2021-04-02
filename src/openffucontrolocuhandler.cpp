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
// -1 written data not maching sent, 0 no issues, 0 < ocuExeptionCode
int OpenFFUcontrolOCUhandler::auxEepromWrite(quint8 slaveAddress, quint32 writeStartAddress, QByteArray data)
{
    // payloads the OCU understands for writing must look like:
    //
    // 4 byte           2 byte      not more than 128 byte
    // start address    bytecount   data

    QByteArray payload;
    QByteArray request;
    ocuResponse response;

    // if addres to write to is not a page start address, existing data must be read and prependet
    // then the EEPROM page can be witten to from the beginning
    // this is an OCU limitation
    if(writeStartAddress % 128 != 0){
        quint32 readFromAddress = writeStartAddress - (writeStartAddress % 128);
        QByteArray preWriteAddressData = auxEepromRead(slaveAddress, readFromAddress, writeStartAddress % 128);
        data.prepend(preWriteAddressData);
    }

    qint16 byteCount = 0;       // number of bytes in next transmision, max 128
    while(data.length() != 0){  // transmit data in max 128 byte parts

        if (data.length() > 128){
            byteCount = 128;
        } else{
            byteCount = data.length();
        }
        payload.append(writeStartAddress);
        payload.append(byteCount);
        payload.append(data.left(byteCount));

        // send data to write to the OCU
        request = createRequest(slaveAddress, OCU_AUX_EEPROM_WRITE, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        // OCU must confirm transmition with ACK
        if (response.exeptionCode != E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromWrite() failed to write: %s", errorString(response.exeptionCode).toLocal8Bit().data());
            return response.exeptionCode;
        }

        // wait for OCU to write page
        for(; systemBusy(slaveAddress) ;){
            QThread::msleep(100);
        }

        //read back and compare written data with sent data
        if (data.left(byteCount) != auxEepromRead(slaveAddress, writeStartAddress, byteCount)){
                fprintf(stderr, "OpneFFUcontrollerOCUhandler::auxEepromWrite(): EEPROM data read back does NOT match data sent.");
                return -1;
        }

        data.remove(0, byteCount);
    }

    return 0;
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

QString OpenFFUcontrolOCUhandler::errorString(quint8 errorCode)
{
    switch (errorCode) {
    case E_ILLEGAL_FUNCTION:
        return "illegal function";
    case E_ILLEGAL_DATA_ADDRESS:
        return "illegal data addres";
    case E_ILLEGAL_DATA_VALUE:
        return "illegal data value";
    case E_SERVER_DEVICE_FAILURE:
        return "sever device failure";
    case E_ACKNOWLEDGE:
        return "acknowledge";
    case E_SERVER_DEVICE_BUSY:
        return "davice busy";
    case E_MEMORY_PARITY_ERROR:
        return "memory parity error";
    case E_GATEWAY_PATH_UNAVAILABLE:
        return "gateway path unavailable";
    case E_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND:
        return "gateway target device faield to respond";
    default:
        break;
    }

    return "Unnknown Error";
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
