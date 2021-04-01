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

    quint8 functionCode;
    bool isDryRun;
    QString modbusInterface;
    QString pathToHexfile;
    QByteArray payload;
    quint8 slaveId;
    QString deviceType;

    void parseArguments(QStringList arguments);
    void executeArguments();

    void parseIntelHex(QString file);

signals:

};

#endif // MAINCONTROLLER_H
