#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QCommandLineParser>
#include <QDebug>
#include <QObject>

#include "intelhexparser.h"
#include "modbus.h"
#include "openffucontrolocuhandler.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent, QStringList arguments);
    ~MainController();

private:
    ModBus* m_modbus;
    OpenFFUcontrolOCUhandler* m_ocuHandler;

    // set by commandline arguments
    qint32 m_baudRate = 9600;
    quint8 m_functionCode = 0;
    bool m_isDryRun = false;
    QString m_modbusInterface;
    QString m_pathToHexfile;
    QByteArray m_payload;
    bool m_directDataSend = false;
    quint8 m_slaveId = 0;
    QString m_deviceType;
    bool m_reset = false;           // reset application to bootloader
    bool m_update = false;
    bool m_erase = false;           // memory erase
    bool m_copy = false;            // memory copy
    QString m_memory = "";          // memory type to apply operations to
    quint32 m_memoryAddress = 0;    // start address for memory operations
    quint64 m_byteCount = 0;        // byte count to read from memory
    bool m_debug = false;

    void parseArguments(QStringList arguments);
    void executeArguments();

    QByteArray getIntelHexContent(QString file);

signals:

};

#endif // MAINCONTROLLER_H
