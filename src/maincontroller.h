#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QCommandLineParser>
#include <QDebug>
#include <QObject>

#include "intelhexparser.h"
#include "modbushandler.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent, QStringList arguments);
    ~MainController();

private:
    ModbusHandler* m_modbushandler;

    bool isDryRun;
    QString modbusInterface;
    QString pathToHexfile;

    void parseArguments(QStringList arguments);
    void executeArguments();

    void parseIntelHex(QString file);

signals:

};

#endif // MAINCONTROLLER_H
