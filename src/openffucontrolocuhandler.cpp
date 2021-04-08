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

    if (response.exeptionCode != E_ACKNOWLEDGE){
        fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromErase() failed: %s\n", errorString(response.exeptionCode).toLocal8Bit().data());
        return false;
    }

    return true;
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

        // assemble payload
        payload.append(writeStartAddress);
        payload.append(byteCount);
        payload.append(data.left(byteCount));

        // send data to write to the OCU
        request = createRequest(slaveAddress, OCU_AUX_EEPROM_WRITE, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        // OCU must confirm transmition with ACK
        if (response.exeptionCode != E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromWrite() failed to write: %s\n", errorString(response.exeptionCode).toLocal8Bit().data());
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

QByteArray OpenFFUcontrolOCUhandler::auxEepromRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 4 byte           2 byte
    // start address    bytecount no more than 128 byte

    QByteArray payload;
    QByteArray request;
    ocuResponse response;
    QByteArray data;

    quint8 requestByteCount = 0;
    quint32 requestReadAddress = 0;
    for (quint64 i = 0; byteCount != 0; i++){
        if (byteCount > 128){
            requestByteCount = 128;
            byteCount = byteCount - 128;
        } else{
            requestByteCount = byteCount;
            byteCount = 0;
        }
        requestReadAddress = readStartAddress + i * 128;

        payload.append(requestReadAddress);
        payload.append(requestByteCount);

        request = createRequest(slaveAddress, OCU_AUX_EEPROM_READ, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        if(response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(response.payload);

        payload.clear();
    }

    return data;
}
// returns 0 when sucsessfull, else OCU exeption code
int OpenFFUcontrolOCUhandler::copyAuxEepromToFlash(quint8 slaveAddress)
{
    QByteArray request = createRequest(slaveAddress, OCU_COPY_EEPROM_TO_FLASH);
    ocuResponse response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

    if (response.exeptionCode == E_ACKNOWLEDGE){
        return 0;
    }

    return response.exeptionCode;
}

QByteArray OpenFFUcontrolOCUhandler::intFlashRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 4 byte           2 byte
    // start address    bytecount no more than 128 byte

    QByteArray payload;
    QByteArray request;
    ocuResponse response;
    QByteArray data;

    quint8 requestByteCount = 0;
    quint32 requestReadAddress = 0;
    for (quint64 i = 0; byteCount != 0; i++){
        if (byteCount > 128){
            requestByteCount = 128;
            byteCount = byteCount - 128;
        } else{
            requestByteCount = byteCount;
            byteCount = 0;
        }
        requestReadAddress = readStartAddress + i * 128;

        payload.append(requestReadAddress);
        payload.append(requestByteCount);

        request = createRequest(slaveAddress, OCU_INT_FLASH_READ, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        if(response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intFlashRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(response.payload);

        payload.clear();
    }

    return data;
}
// -1 written data not maching sent, 0 no issues, 0 < ocuExeptionCode
int OpenFFUcontrolOCUhandler::intEepromWrite(quint8 slaveAddress, quint16 writeStartAddress, QByteArray data)
{
    // payloads the OCU understands for writing must look like:
    //
    // 2 byte           2 byte      not more than 128 byte
    // start address    bytecount   data

    QByteArray payload;
    QByteArray request;
    ocuResponse response;

    // if addres to write to is not a page start address, existing data must be read and prependet
    // then the EEPROM page can be witten to from the beginning
    // this is an OCU limitation
    if(writeStartAddress % 128 != 0){
        quint16 readFromAddress = writeStartAddress - (writeStartAddress % 128);
        QByteArray preWriteAddressData = intEepromRead(slaveAddress, readFromAddress, writeStartAddress % 128);
        data.prepend(preWriteAddressData);
    }

    qint16 byteCount = 0;       // number of bytes in next transmision, max 128
    while(data.length() != 0){  // transmit data in max 128 byte parts

        if (data.length() > 128){
            byteCount = 128;
        } else{
            byteCount = data.length();
        }

        // assemble payload
        payload.append(writeStartAddress);
        payload.append(byteCount);
        payload.append(data.left(byteCount));

        // send data to write to the OCU
        request = createRequest(slaveAddress, OCU_INT_EEPROM_WRITE, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        // OCU must confirm transmition with ACK
        if (response.exeptionCode != E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromWrite() failed to write: %s\n", errorString(response.exeptionCode).toLocal8Bit().data());
            return response.exeptionCode;
        }

        // wait for OCU to write page
        for(; systemBusy(slaveAddress) ;){
            QThread::msleep(100);
        }

        //read back and compare written data with sent data
        if (data.left(byteCount) != intEepromRead(slaveAddress, writeStartAddress, byteCount)){
                fprintf(stderr, "OpneFFUcontrollerOCUhandler::intEepromWrite(): EEPROM data read back does NOT match data sent.");
                return -1;
        }

        data.remove(0, byteCount);
    }

    return 0;
}

QByteArray OpenFFUcontrolOCUhandler::intEepromRead(quint8 slaveAddress, quint16 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 2 byte           2 byte
    // start address    bytecount no more than 128 byte

    QByteArray payload;
    QByteArray request;
    ocuResponse response;
    QByteArray data;

    quint8 requestByteCount = 0;
    quint32 requestReadAddress = 0;
    for (quint64 i = 0; byteCount != 0; i++){
        if (byteCount > 128){
            requestByteCount = 128;
            byteCount = byteCount - 128;
        } else{
            requestByteCount = byteCount;
            byteCount = 0;
        }
        requestReadAddress = readStartAddress + i * 128;

        payload.append(requestReadAddress);
        payload.append(requestByteCount);

        request = createRequest(slaveAddress, OCU_INT_FLASH_READ, payload);
        response = parseOCUResponse(m_modbusHander->sendRawRequest(request));

        if(response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intFlashRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(response.payload);

        payload.clear();
    }

    return data;
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
    case E_PARSER_FAILED:
        return "response parser failed";
    default:
        break;
    }

    return "Unnknown Error";
}
// returns true if update was written succesfully
bool OpenFFUcontrolOCUhandler::updateFirmware(quint8 slaveAddress, QByteArray application)
{
    // erase aux EEPROM before writng new firmware
    fprintf(stdout, "Erasing auxiliary EEPROM\n");
    if (!auxEepromErase(slaveAddress)){
        return false;
    }
    // write update to aux EEPROM
    fprintf(stdout, "Writing update to auxiliary EEPROM\n");
    if(auxEepromWrite(slaveAddress, 0, application) != 0){
        return false;
    }
    // copy EEPROM into programm flash
    fprintf(stdout, "Copy auxiliary EEPROM to program flash\n");
    if (copyAuxEepromToFlash(slaveAddress) != 0){
        return false;
    }
    return true;
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
            fprintf(stderr, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Failed to parse resonse. Response thought to be an OCU exeption but payload lengt is not 1.\n");
            parsed.exeptionCode = E_PARSER_FAILED;
        } else {
            parsed.exeptionCode = response.at(2);
        }
    }

    return parsed;
}
