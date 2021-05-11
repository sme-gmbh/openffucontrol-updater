#include "openffucontrolocuhandler.h"

OpenFFUcontrolOCUhandler::OpenFFUcontrolOCUhandler(QObject *parent, ModBus *modbus, bool dryRun, bool debug)
{
    m_modbus = modbus;
    isDryRun = dryRun;
    isDebug = debug;
}

// returns ocuExeptionCode 0 if only data was sent
quint8 OpenFFUcontrolOCUhandler::sendRawCommand(quint8 slaveAddress, quint16 functonCode, QByteArray payload)
{
    if (isDebug)
        fprintf(stdout, "OpenFFUcontrolOCUhandler::sendRawCommand() sending to slave %i functioncode %i payload %s\n", slaveAddress, functonCode, payload.toHex().data());
    m_response = parseOCUResponse( m_modbus->sendRawRequestBlocking(slaveAddress, functonCode, payload));
    return m_response.exeptionCode;
}

bool OpenFFUcontrolOCUhandler::auxEepromErase(quint8 slaveAddress)
{
    //QByteArray request = createRequest(slaveAddress, OCU_AUX_EEPROM_ERASE);
    sendRawCommand(slaveAddress, OCU_AUX_EEPROM_ERASE, QByteArray());

    if (m_response.exeptionCode != E_ACKNOWLEDGE){
        fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromErase() failed: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
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

    // if addres to write to is not a page start address, existing data must be read and prepended
    // then the EEPROM page can be witten to from the beginning
    // this is an OCU limitation
    if(writeStartAddress % 128 != 0){
        quint32 readFromAddress = writeStartAddress - (writeStartAddress % 128);
        QByteArray preWriteAddressData = auxEepromRead(slaveAddress, readFromAddress, writeStartAddress % 128 - 1);
        data.prepend(preWriteAddressData);
        writeStartAddress = readFromAddress;    // set start address to match new data + write data
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
        sendRawCommand(slaveAddress, OCU_AUX_EEPROM_WRITE, payload);

        // OCU must confirm transmition with ACK
        if (m_response.exeptionCode != E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromWrite() failed to write: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
            return m_response.exeptionCode;
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

        sendRawCommand(slaveAddress, OCU_AUX_EEPROM_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(m_response.payload);

        payload.clear();
    }

    return data;
}

int OpenFFUcontrolOCUhandler::intFlashErase(quint8 slaveAddress)
{
    return E_ILLEGAL_FUNCTION;
}
// returns 0 when sucsessfull, else OCU exeption code
int OpenFFUcontrolOCUhandler::copyAuxEepromToFlash(quint8 slaveAddress)
{
    sendRawCommand(slaveAddress, OCU_COPY_EEPROM_TO_FLASH, QByteArray());

    if (m_response.exeptionCode == E_ACKNOWLEDGE){
        return 0;
    }

    return m_response.exeptionCode;
}

QByteArray OpenFFUcontrolOCUhandler::intFlashRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 4 byte           2 byte
    // start address    bytecount no more than 128 byte

    QByteArray payload;
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

        sendRawCommand(slaveAddress, OCU_INT_FLASH_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intFlashRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(m_response.payload);

        payload.clear();
    }

    return data;
}
// -1 written data not maching sent, 0 no issues, 0 < ocuExeptionCode
int OpenFFUcontrolOCUhandler::intEepromWrite(quint8 slaveAddress, quint32 writeStartAddress, QByteArray data)
{
    // payloads the OCU understands for writing must look like:
    //
    // 4 byte           2 byte      not more than 128 byte
    // start address    bytecount   data

    QByteArray payload;

    // if addres to write to is not a page start address, existing data must be read and prepended
    // then the EEPROM page can be witten to from the beginning
    // this is an OCU limitation
    if(writeStartAddress % 128 != 0){
        quint32 readFromAddress = writeStartAddress - (writeStartAddress % 128);
        QByteArray preWriteAddressData = auxEepromRead(slaveAddress, readFromAddress, writeStartAddress % 128 - 1);
        data.prepend(preWriteAddressData);
        writeStartAddress = readFromAddress;    // set start address to match new data + write data
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
        sendRawCommand(slaveAddress, OCU_INT_EEPROM_WRITE, payload);

        // OCU must confirm transmition with ACK
        if (m_response.exeptionCode != E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intEepromWrite() failed to write: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
            return m_response.exeptionCode;
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

QByteArray OpenFFUcontrolOCUhandler::intEepromRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 4 byte           2 byte
    // start address    bytecount no more than 128 byte

    QByteArray payload;
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

        sendRawCommand(slaveAddress, OCU_INT_FLASH_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intFlashRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(m_response.payload);

        payload.clear();
    }

    return data;
}
// returns true if system is busy
bool OpenFFUcontrolOCUhandler::systemBusy(quint8 slaveAddress)
{
    sendRawCommand(slaveAddress, OCU_STATUS_READ, QByteArray());

    if (m_response.exeptionCode == E_ACKNOWLEDGE){
        return false;
    }

    return true;
}
// Requests ocu application boot. OCU does not confirm the success.
void OpenFFUcontrolOCUhandler::bootApplication(quint8 slaveAddress)
{
    sendRawCommand(slaveAddress, OCU_AUX_EEPROM_ERASE, QByteArray());
}

QString OpenFFUcontrolOCUhandler::errorString(quint8 errorCode)
{
    switch (errorCode) {
    case E_UNKNOWN_ERROR:
        return "unknown error";
    case E_NO_ERROR:
        return "no error";
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

    return "Unknown Error";
}

QByteArray OpenFFUcontrolOCUhandler::getResponsePayload()
{
    return m_response.payload;
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

    if (isDebug){
        QByteArray buffer = response;
        fprintf(stdout, "DEBUG OpenFFUcontrolOCUhandler::parseOCUResponse(): Response to parse: 0x%s\n", buffer.toHex().data());
    }

    if (response == nullptr && isDryRun){
        parsed.exeptionCode = E_ACKNOWLEDGE;
        return parsed;
    }

    if (response.isEmpty()){
        fprintf(stderr, "OpenFFUcontrollerOCUhandler::parseOCUResponse(): got empty response to parse\n");
        parsed.exeptionCode = E_PARSER_FAILED;
        return parsed;
    }

    parsed.slaveId = response.at(0);
    parsed.functionCode = response.at(1);
    parsed.payload = response.mid(2, response.length() - 4);
    parsed.crc = response.right(2).toUShort(nullptr, 10);


    if (isDebug){
        fprintf(stdout, "DEBUG Parsed slave ID: %i\n", parsed.slaveId);
        fprintf(stdout, "DEBUG Parsed function code: %i\n", parsed.functionCode);
        fprintf(stdout, "DEBUG Parsed payload: 0x%s\n", parsed.payload.toHex().data());
        fprintf(stdout, "DEBUG Parsed crc: %i\n", parsed.crc);
    }

    // exeption responses use reqest_functionCode + 0x80 as indication for errors
    if (parsed.functionCode >= 0x80){
        // exeption codes are only one byte, if there are more there must be an issue in parsing
        if (parsed.payload.length() != 1){
            fprintf(stderr, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Failed to parse resonse. Response thought to be an OCU exeption but payload lengt is not 1.\n");
            parsed.exeptionCode = E_PARSER_FAILED;
        } else {
            parsed.exeptionCode = response.at(2);
            fprintf(stdout, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Error code received: %i\n", parsed.exeptionCode);
        }
    }

    return parsed;
}
