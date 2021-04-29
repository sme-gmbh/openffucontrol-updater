#include "modbus.h"

ModBus::ModBus(QObject *parent, QString interface, bool debug) : QObject(parent)
{
    m_interface = interface;
    m_debug = debug;
    m_port = new QSerialPort(interface, this);
    m_transactionPending = false;
    m_currentTelegram = NULL;
    m_rx_telegrams = 0;
    m_crc_errors = 0;

    // This timer notifies about a telegram timeout if a unit does not answer
    m_requestTimer.setSingleShot(true);
    m_requestTimer.setInterval(200);
    connect(&m_requestTimer, SIGNAL(timeout()), this, SLOT(slot_requestTimer_fired()));

    // This timer delays tx after rx to wait for line clearance
    m_delayTxTimer.setSingleShot(true);
    m_delayTxTimer.setInterval(4);
    connect(this, SIGNAL(signal_transactionFinished()), &m_delayTxTimer, SLOT(start()));
    connect(&m_delayTxTimer, SIGNAL(timeout()), this, SLOT(slot_tryToSendNextTelegram()));

    // This timer fires if receiver does not get any more bytes and telegram should be complete
    m_rxIdleTimer.setSingleShot(true);
    m_rxIdleTimer.setInterval(50);
    connect(&m_rxIdleTimer, SIGNAL(timeout()), this, SLOT(slot_rxIdleTimer_fired()));
}

ModBus::~ModBus()
{
    if (m_port->isOpen())
        this->close();
    delete m_port;
}

bool ModBus::open(qint32 baudrate)
{
    m_port->setBaudRate(baudrate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::TwoStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    connect(m_port, SIGNAL(readyRead()), this, SLOT(slot_readyRead()));
    bool openOK = m_port->open(QIODevice::ReadWrite);
    m_port->setBreakEnabled(false);
    m_port->setTextModeEnabled(false);

    return openOK;
}

void ModBus::close()
{
    if (m_port->isOpen())
        m_port->close();
}

quint64 ModBus::sendRawRequest(quint8 slaveAddress, quint8 functionCode, QByteArray payload)
{
    return writeTelegramToQueue(new ModBusTelegram(slaveAddress, functionCode, payload));
}

QByteArray ModBus::sendRawRequestBlocking(quint8 slaveAddress, quint8 functionCode, QByteArray payload)
{
    QSignalSpy spy(this, SIGNAL(signal_responseRawComplete(quint64, QByteArray)));

    writeTelegramToQueue(new ModBusTelegram(slaveAddress, functionCode, payload));
    QByteArray response;
    if (spy.wait(10000))
    {
        QList<QVariant> arguments = spy.takeFirst();
        response = arguments.at(1).toByteArray();
    }

    return response;
}

quint64 ModBus::readRegisters(quint8 slaveAddress, quint16 dataStartAddress, quint8 count)
{

}

quint64 ModBus::writeRegisters(quint8 slaveAddress, quint16 dataStartAddress, QList<quint16> data)
{

}

quint16 ModBus::readRegister(quint8 slaveAddress, quint16 dataAddress)
{

}

quint16 ModBus::writeRegister(quint8 slaveAddress, quint16 dataAddress, quint16 data)
{

}

quint64 ModBus::readDiscreteInput(quint8 slaveAddress, quint16 dataAddress)
{

}

quint64 ModBus::writeCoil(quint8 slaveAddress, quint16 dataAddress, quint8 bit, bool data)
{

}

void ModBus::slot_tryToSendNextTelegram()
{
    m_telegramQueueMutex.lock();
    // Delete last telegram if it exists
    // If repeat counter is not zero, then repeat current telegram, otherwise take new
    // telegram from the queue
    if ((m_currentTelegram != NULL) && (m_currentTelegram->repeatCount == 0))
    {
        delete m_currentTelegram;
        m_currentTelegram = NULL;
    }

    if (m_currentTelegram == NULL)
    {
        if (m_telegramQueue.isEmpty())
        {
            m_telegramQueueMutex.unlock();
            return;
        }
        m_currentTelegram = m_telegramQueue.takeFirst();
    }

    m_requestTimer.start();
    m_telegramQueueMutex.unlock();

    writeTelegramNow(m_currentTelegram);
}

quint64 ModBus::writeTelegramToQueue(ModBusTelegram *telegram)
{
    quint64 telegramID = telegram->getID();
    m_telegramQueueMutex.lock();
    m_telegramQueue.append(telegram);

    if (!m_requestTimer.isActive()) // If we inserted the first packet, we have to start the sending process
    {
        m_telegramQueueMutex.unlock();
        slot_tryToSendNextTelegram();
    }
    else
        m_telegramQueueMutex.unlock();

    return telegramID;
}

quint64 ModBus::rx_telegrams() const
{
    return m_rx_telegrams;
}

quint64 ModBus::crc_errors() const
{
    return m_crc_errors;
}

quint64 ModBus::writeTelegramNow(ModBusTelegram *telegram)
{
    quint8 slaveAddress = telegram->slaveAddress;
    quint8 functionCode = telegram->functionCode;
    QByteArray data = telegram->data;
    telegram->repeatCount--;

    writeTelegramRawNow(slaveAddress, functionCode, data);
    return telegram->getID();
}

void ModBus::writeTelegramRawNow(quint8 slaveAddress, quint8 functionCode, QByteArray data)
{
    QByteArray out;

    out.append(slaveAddress);
    out.append(functionCode);
    out.append(data);

    quint16 cs = checksum(out);
    out.append(cs & 0xFF);
    out.append(cs >> 8);

    if (m_port->isOpen())
    {
        if (m_debug)
        {
            fprintf(stdout, "ModBus::writeTelegramRawNow: Writing: %s\n", out.toHex().data());
            fflush(stdout);
        }
        m_port->write(out);
        m_port->flush();
    }
}

void ModBus::tryToParseResponseRaw(QByteArray *buffer)
{
    if (buffer->size() < 4)
        return;

    if (m_debug)
    {
        fprintf(stdout, "ModBus::tryToParseResponseRaw: Reading: %s\n", buffer->toHex().data());
        fflush(stdout);
    }

    quint8 address = buffer->at(0);
    quint8 functionCode = buffer->at(1) & 0x7F;
    bool exception = buffer->at(1) & 0x80;

    // Check address match here in case we are Modbus server
    // if(server)
    // {...}

    if (exception)
    {
        if (buffer->size() < 5)
            return;

        quint8 exceptionCode = buffer->at(2);
        if (!checksumOK(*buffer))
        {
            buffer->clear();
            m_crc_errors++;
            return;
        }

        m_requestTimer.stop();
        m_rx_telegrams++;
        // Parse exception here and send signal!
        emit signal_exception(m_currentTelegram->getID(), exceptionCode);
        emit signal_responseRawComplete(m_currentTelegram->getID(), *buffer);
        emit signal_transactionFinished();
        buffer->clear();
        return;
    }

    // Try to check length here (todo!)
    if (buffer->length() > 255)
    {
        buffer->clear();
        return;
    }

    if (!checksumOK(*buffer))
    {
        buffer->clear();
        m_crc_errors++;
        return;
    }

    // If we reach this code, checksum is ok

    m_requestTimer.stop();
    m_rx_telegrams++;
    QByteArray data = buffer->mid(2, buffer->length() - 4); // Fill data with PDU

    emit signal_responseRawComplete(m_currentTelegram->getID(), *buffer);
    emit signal_responseRaw(m_currentTelegram->getID(), address, functionCode, data);
    parseResponse(m_currentTelegram->getID(), address, functionCode, data);
    emit signal_transactionFinished();

    buffer->clear();
}

void ModBus::parseResponse(quint64 id, quint8 slaveAddress, quint8 functionCode, QByteArray data)
{

}

quint16 ModBus::checksum(QByteArray data)
{
    quint16 crc = 0xffff;
    quint16 i;

    for (i=0;i < data.length(); i++)
    {
        const uint16_t polynom = 0xA001;

        bool c;
        uint8_t crc_hi, crc_low;

        crc_hi = crc >> 8;
        crc_low = (crc & 0xFF) ^ data.at(i);
        crc = (crc_hi << 8) | crc_low;

        for (quint8 j=0; j<=7; j++)
        {
            c = (crc & 0x001);
            crc = (crc >> 1);
            if (c) crc ^= polynom;
        }
    }

    return crc;
}

bool ModBus::checksumOK(QByteArray data)
{
    quint16 crc = 0;
    crc =  data.at(data.length() - 2);
    crc += data.at(data.length() - 1) << 8;

    data.remove(data.length() - 2, 2);

    return (checksum(data) == crc);
}

void ModBus::slot_readyRead()
{
    while (!m_port->atEnd())
    {
        char c;
        m_port->getChar(&c);
        m_readBuffer.append(c);
        m_rxIdleTimer.start();  // Start resets the timer even if it has not finished in order to run the full time again
    }
}

void ModBus::slot_requestTimer_fired()
{
    if (m_currentTelegram->needsAnswer())
    {
        emit signal_transactionLost(m_currentTelegram->getID());
    }
    else
    {
        emit signal_transactionFinished();
    }
}

void ModBus::slot_rxIdleTimer_fired()
{
    tryToParseResponseRaw(&m_readBuffer);
}
