#include "maincontroller.h"
#include <QTextStream>

MainController::MainController(QObject *parent, QStringList arguments) : QObject(parent)
{
    fprintf(stdout, " --- openFFUcontrol-updater ---\n");

    parseArguments(arguments);
    executeArguments();

}

MainController::~MainController()
{
}

void MainController::parseArguments(QStringList arguments)
{
    QCommandLineParser parser;
       parser.setApplicationDescription("openFFUcontrol-updater");
       parser.addHelpOption();

       parser.addOptions({
                             // function code for modbus command (-c, --functionCode)
                             {{"c","functionCode"},
                                 QCoreApplication::translate("main", "function code sent to slaveID as uint8"),
                                 QCoreApplication::translate("main", "function code")
                             },
                             // do not write to modbus (-d, --dry-run)
                             {{"d","dry-run"},
                                 QCoreApplication::translate("main", "does not acces the Modbus interface")
                             },
                             // modbus interface to use (-i, --interface)
                             {{"i","interface"},
                                 QCoreApplication::translate("main", "Modbus interface name to use"),
                                 QCoreApplication::translate("main", "interface name")
                             },
                             // path to hexfile to be useds);
                             {{"hf", "hexfile"},
                                 QCoreApplication::translate("main", "Hex file to use"),
                                 QCoreApplication::translate("main", "file")
                             },
                             // data payload sent to slaveID under the function code (-p, --payload)
                             {{"p","payload"},
                                 QCoreApplication::translate("main", "data payload sent to --slaveID under --functionCode, data must be in hex without leading 0x and fit the function code, else corruption might occure. There are no checks to make shure the data fits to the function code!"),
                                 QCoreApplication::translate("main", "data")
                             },
                             // device type to update
                             {{"s", "slave", "slave-ID"},
                                 QCoreApplication::translate("main", "Slave address as uint8 used to connect to --type device on --interface interface"),
                                 QCoreApplication::translate("main", "address")
                             },
                             // device type to update
                             {{"t", "type"},
                                 QCoreApplication::translate("main", "Device type to update via modbus. Defaults to OCU. Supported: OCU"),
                                 QCoreApplication::translate("main", "device type")
                             },

       });

       // Process the actual command line arguments given by the user
       parser.process(arguments);

       functionCode = parser.value("functionCode").toUInt();
       isDryRun = parser.isSet("dry-run");
       pathToHexfile = parser.value("hexfile");
       modbusInterface = parser.value("interface");
       payload = QByteArray::fromHex( parser.value("payload").toLocal8Bit());
       slaveId = parser.value("slave").toUInt();
       deviceType = parser.value("type");
}
// execute what is commanded by the user
void MainController::executeArguments()
{
    if (!pathToHexfile.isEmpty())
       parseIntelHex(pathToHexfile);
    if (!modbusInterface.isEmpty()){
        m_modbushandler = new ModbusHandler(this, modbusInterface, isDryRun);
        m_modbushandler->open();
    }

    if (deviceType == "OCU" || deviceType.isEmpty()){
        m_ocuHandler = new OpenFFUcontrolOCUhandler(this, m_modbushandler);
        if (!payload.isEmpty()){
            qDebug() << "slave address: " << slaveId << " function code: " << functionCode << " payload: " << payload.toHex();
            m_ocuHandler->sendRawCommand(slaveId, functionCode, payload);
        }
    }
}

void MainController::parseIntelHex(QString file)
{
    IntelHexParser parser;
    parser.parse(file);
}
