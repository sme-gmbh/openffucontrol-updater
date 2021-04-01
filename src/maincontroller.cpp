#include "maincontroller.h"
#include <QTextStream>

MainController::MainController(QObject *parent, QStringList arguments) : QObject(parent)
{
    fprintf(stdout, " --- openFFUupdater ---\n");

    parseArguments(arguments);
    executeArguments();

}

MainController::~MainController()
{
}

void MainController::parseArguments(QStringList arguments)
{
//    QString
//    for(qint8 i = 0; i < arguments.length(); i++){

//    }

    QCommandLineParser parser;
       parser.setApplicationDescription("Test helper");
       parser.addHelpOption();

       parser.addOptions({
                             // modbus interface to use (-d, --dry-run)
                             {{"d","dry-run"},
                                 QCoreApplication::translate("main", "does not acces the Modbus interface !!!NOT WORKING!!!!")
                             },

                             // modbus interface to use (-i, --interface)
                             {{"i","interface"},
                                 QCoreApplication::translate("main", "Modbus interface name to use")
                             },

                             // path to hexfile to be used
                             {{"hf", "hexfile"},
                                 QCoreApplication::translate("main", "Hex file to use"),
                                 QCoreApplication::translate("main", "directory")
                             },

       });

       // Process the actual command line arguments given by the user
       parser.process(arguments);

       isDryRun = parser.isSet("dry-run");
       qDebug() << "Is dry run: " << isDryRun;
       pathToHexfile = parser.value("hexfile");
       qDebug() << "Path to hex file: " << pathToHexfile;
       modbusInterface = parser.value("interface");
       qDebug() << "Modbus interface: " << modbusInterface;
}
// execute what is commanded by the user
void MainController::executeArguments()
{
    if (!pathToHexfile.isEmpty())
       parseIntelHex(pathToHexfile);
    if (!modbusInterface.isEmpty())
        m_modbushandler = new ModbusHandler(this, modbusInterface);
}

void MainController::parseIntelHex(QString file)
{
    IntelHexParser parser;
    parser.parse(file);
}
