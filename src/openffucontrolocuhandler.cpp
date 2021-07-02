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

    if (m_response.exeptionCode != ModBusTelegram::E_ACKNOWLEDGE){
        fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromErase() failed: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
        return false;
    }

    waitForOCU(slaveAddress);

    return true;
}
// -1 written data not maching sent, 0 no issues, 0 < ocuExeptionCode
int OpenFFUcontrolOCUhandler::auxEepromWrite(quint8 slaveAddress, quint32 writeStartAddress, QByteArray data)
{
    // payloads the OCU understands for writing must look like:
    //
    // 4 byte           2 byte      not more than 128 byte
    // start address    bytecount   data

    // if addres to write to is not a page start address, existing data must be read and prepended
    // then the EEPROM page can be witten to from the beginning
    // this is an OCU limitation
    if(writeStartAddress % 128 != 0){
        fprintf(stdout, "Writing %5i bytes non-aligned at 0x%06x\n", data.length(), writeStartAddress);
        quint32 readFromAddress = writeStartAddress - (writeStartAddress % 128);
        QByteArray preWriteAddressData = auxEepromRead(slaveAddress, readFromAddress, writeStartAddress % 128 - 1);
        data.prepend(preWriteAddressData);
        writeStartAddress = readFromAddress;    // set start address to match new data + write data
    } else {
        fprintf(stdout, "Writing %5i bytes aligned at 0x%06x\n", data.length(), writeStartAddress);
    }

    qint16 byteCount = 0;       // number of bytes in next transmision, max 128
    while(data.length() != 0){  // transmit data in max 128 byte parts

        if (data.length() > 128){
            byteCount = 128;
        } else{
            byteCount = data.length();
        }

        QByteArray payload;
        payload = assembleAddressHeader(writeStartAddress, byteCount);
        payload += data.left(byteCount);

        // send data to write to the OCU
        sendRawCommand(slaveAddress, OCU_AUX_EEPROM_WRITE, payload);

        // OCU must confirm transmition with ACK
        if (m_response.exeptionCode != ModBusTelegram::E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromWrite() failed to write: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
            return m_response.exeptionCode;
        }

        waitForOCU(slaveAddress);

        //read back and compare written data with sent data
        if (data.left(byteCount) != auxEepromRead(slaveAddress, writeStartAddress, byteCount)){
                fprintf(stderr, "OpenFFUcontrollerOCUhandler::auxEepromWrite(): EEPROM data read back does NOT match data sent.\n");
                return -1;
        } else {
            fprintf(stdout, "Write success at 0x%06x, data: %s\n", writeStartAddress, data.left(byteCount).toHex().data());
            writeStartAddress += byteCount;
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

        QByteArray payload;
        payload = assembleAddressHeader(requestReadAddress, requestByteCount);

        sendRawCommand(slaveAddress, OCU_AUX_EEPROM_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::auxEepromRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(m_response.payload);
    }

    return data;
}

int OpenFFUcontrolOCUhandler::intFlashErase(quint8 slaveAddress)
{
    Q_UNUSED(slaveAddress)
    return ModBusTelegram::E_ILLEGAL_FUNCTION;
}

// returns 0 when sucsessfull, else OCU exeption code
int OpenFFUcontrolOCUhandler::copyAuxEepromToFlash(quint8 slaveAddress)
{
    sendRawCommand(slaveAddress, OCU_COPY_EEPROM_TO_FLASH, QByteArray());

    if (m_response.exeptionCode == ModBusTelegram::E_ACKNOWLEDGE){
        fprintf(stderr, "Copy to Flash has started.\n");
        waitForOCU(slaveAddress);
        fprintf(stderr, "Copy to Flash has finished.\n");
        return 0;
    }

    waitForOCU(slaveAddress);

    return m_response.exeptionCode;
}

QByteArray OpenFFUcontrolOCUhandler::intFlashRead(quint8 slaveAddress, quint32 readStartAddress, quint64 byteCount)
{
    // payloads the OCU understands for reading must look like:
    //
    // 4 byte           2 byte
    // start address    bytecount no more than 128 byte

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

        QByteArray payload;
        payload = assembleAddressHeader(requestReadAddress, requestByteCount);

        sendRawCommand(slaveAddress, OCU_INT_FLASH_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intFlashRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
            return NULL;
        }
        data.append(m_response.payload);
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

        QByteArray payload;
        payload = assembleAddressHeader(writeStartAddress, byteCount);
        payload += data.left(byteCount);

        // send data to write to the OCU
        sendRawCommand(slaveAddress, OCU_INT_EEPROM_WRITE, payload);

        // OCU must confirm transmition with ACK
        if (m_response.exeptionCode != ModBusTelegram::E_ACKNOWLEDGE){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intEepromWrite() failed to write: %s\n", errorString(m_response.exeptionCode).toLocal8Bit().data());
            return m_response.exeptionCode;
        }

        waitForOCU(slaveAddress);

        //read back and compare written data with sent data
        if (data.left(byteCount) != intEepromRead(slaveAddress, writeStartAddress, byteCount)){
                fprintf(stderr, "OpenFFUcontrollerOCUhandler::intEepromWrite(): EEPROM data read back does NOT match data sent.\n");
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

        QByteArray payload;
        payload = assembleAddressHeader(requestReadAddress, requestByteCount);

        sendRawCommand(slaveAddress, OCU_INT_EEPROM_READ, payload);

        if(m_response.exeptionCode != 0){
            fprintf(stderr, "OpenFFUcontrollOCUhandler::intEepromRead() failed to read %i bytes at address %i: %s\n", requestByteCount, requestReadAddress, errorString(m_response.exeptionCode).toLocal8Bit().data());
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

    if (m_response.exeptionCode == ModBusTelegram::E_ACKNOWLEDGE){
        return false;
    }

    return true;
}

// Requests ocu application boot. OCU does not confirm the success.
void OpenFFUcontrolOCUhandler::bootApplication(quint8 slaveAddress)
{
    sendRawCommand(slaveAddress, OCU_BOOT_APPLICATION, QByteArray());
}

QString OpenFFUcontrolOCUhandler::errorString(quint8 errorCode)
{
    switch (errorCode) {
    case E_UNKNOWN_ERROR:
        return "unknown error";
    case E_NO_ERROR:
        return "no error";
    case ModBusTelegram::E_ILLEGAL_FUNCTION:
        return "illegal function";
    case ModBusTelegram::E_ILLEGAL_DATA_ADDRESS:
        return "illegal data addres";
    case ModBusTelegram::E_ILLEGAL_DATA_VALUE:
        return "illegal data value";
    case ModBusTelegram::E_SERVER_DEVICE_FAILURE:
        return "sever device failure";
    case ModBusTelegram::E_ACKNOWLEDGE:
        return "acknowledge";
    case ModBusTelegram::E_SERVER_DEVICE_BUSY:
        return "davice busy";
    case ModBusTelegram::E_MEMORY_PARITY_ERROR:
        return "memory parity error";
    case ModBusTelegram::E_GATEWAY_PATH_UNAVAILABLE:
        return "gateway path unavailable";
    case ModBusTelegram::E_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND:
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
    fprintf(stdout, "Writing update to auxiliary EEPROM at slave %i\n", slaveAddress);
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
        parsed.exeptionCode = ModBusTelegram::E_ACKNOWLEDGE;
        return parsed;
    }

    if (response.isEmpty()){
        if (isDebug)
            fprintf(stdout, "OpenFFUcontrollerOCUhandler::parseOCUResponse(): got empty response to parse\n");
        parsed.exeptionCode = E_PARSER_FAILED;
        return parsed;
    }

    parsed.slaveId = (quint8)response.at(0);
    parsed.functionCode = (quint8)response.at(1) & 0x7F;
    parsed.payload = response.mid(2, response.length() - 4);
//    parsed.crc = response.right(2).toUShort(nullptr, 10);


    if (isDebug){
        fprintf(stdout, "DEBUG Parsed slave ID: %i\n", parsed.slaveId);
        fprintf(stdout, "DEBUG Parsed function code: %i\n", parsed.functionCode);
        fprintf(stdout, "DEBUG Parsed payload: 0x%s\n", parsed.payload.toHex().data());
//        fprintf(stdout, "DEBUG Parsed crc: %i\n", parsed.crc);
    }

    // exeption responses use request_functionCode + 0x80 as indication for errors
    if ((quint8)response.at(1) & 0x80){
        // exeption codes are only one byte, if there are more there must be an issue in parsing
        if (parsed.payload.length() != 1){
            fprintf(stderr, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Failed to parse resonse. Response thought to be an OCU exeption but payload lengt is not 1.\n");
            parsed.exeptionCode = E_PARSER_FAILED;
        } else {
            parsed.exeptionCode = (quint8)response.at(2);
            if (isDebug) {
                fprintf(stdout, "OpenFFUcontrolOCUhandler::parseOCUResponse(): Exception code received: %s\n", errorString(parsed.exeptionCode).toUtf8().data());
            }
        }
    }

    return parsed;
}

QByteArray OpenFFUcontrolOCUhandler::assembleAddressHeader(quint64 startAddress, quint16 byteCount)
{
    QByteArray a;
    a.append((startAddress >> 24) & 0xFF);
    a.append((startAddress >> 16) & 0xFF);
    a.append((startAddress >>  8) & 0xFF);
    a.append((startAddress >>  0) & 0xFF);
    a.append((byteCount >> 8) & 0xFF);
    a.append((byteCount >> 0) & 0xFF);
    return a;
}

void OpenFFUcontrolOCUhandler::waitForOCU(quint8 slaveAddress)
{
    fprintf(stdout, "Waiting for OCU.");
    while(systemBusy(slaveAddress)){
        QThread::msleep(2000);
        fprintf(stdout, ".");
    }
    fprintf(stdout, "\nOCU ready.\n");
}
