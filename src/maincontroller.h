#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QCommandLineParser>
#include <QDebug>
#include <QObject>

#include "intelhexparser.h"
#include "modbushandler.h"
#include "openffucontrolocuhandler.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent, QStringList arguments);
    ~MainController();

private:
    ModbusHandler* m_modbushandler;
    OpenFFUcontrolOCUhandler* m_ocuHandler;

    // set by commandline arguments
    quint16 baudRate = 9600;
    quint8 functionCode = 0;
    bool isDryRun = false;
    QString modbusInterface;
    QString pathToHexfile;
    QByteArray payload;
    bool directDataSend = false;
    quint8 slaveId = 0;
    QString deviceType;
//    quint32 mbusByteTimeoutSec = 0;
//    quint32 mbusByteTimeoutMSec = 0;
    bool update = false;
    bool debug = false;

    void parseArguments(QStringList arguments);
    void executeArguments();

    QByteArray getIntelHexContent(QString file);

signals:

};

#endif // MAINCONTROLLER_H
