#include "maincontroller.h"
#include <QTextStream>

MainController::MainController(QObject *parent, QStringList arguments) : QObject(parent)
{
    fprintf(stdout, " --- openFFUcontrol-updater ---\n\n");

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
                             {{"b","baud"},
                                 QCoreApplication::translate("main", "baudrate of the modbus interface"),
                                 QCoreApplication::translate("main", "rate")
                             },
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
                             // send direct user command
                             {{"r","direct-command"},
                                 QCoreApplication::translate("main", "Sends function code and payload (if set) to slave"),
                             },
                             // device type to update
                             {{"s", "slave", "slave-ID"},
                                 QCoreApplication::translate("main", "Slave address as uint8 used to connect to --type device on --interface interface. Defaults to 0."),
                                 QCoreApplication::translate("main", "address")
                             },
                             // device type to interface
                             {{"t", "type"},
                                 QCoreApplication::translate("main", "Device type to inteface via modbus. Defaults to OCU. Supported: OCU"),
                                 QCoreApplication::translate("main", "device type")
                             },// modbus timout
//                             {{"bts", "byte-timeout-sec"},
//                                 QCoreApplication::translate("main", "Modbus byte timeout in seconds"),
//                                 QCoreApplication::translate("main", "timeout")
//                             },{{"btms", "byte-timeout-msec"},
//                                QCoreApplication::translate("main", "Modbus byte timeout in milliesconds"),
//                                QCoreApplication::translate("main", "timeout")
//                            },
                             // fully automatic update of device
                             {{"u", "update"},
                                 QCoreApplication::translate("main", "Updates device using the hex file via the modbus interface"),
                             },
                             // show debug output
                             {{"v", "debug"},
                                 QCoreApplication::translate("main", "show debug output"),
                             },

       });

       // Process the actual command line arguments given by the user
       parser.process(arguments);

       baudRate = parser.value("baud").toShort();
       functionCode = parser.value("functionCode").toUInt();
       isDryRun = parser.isSet("dry-run");
       pathToHexfile = parser.value("hexfile");
       modbusInterface = parser.value("interface");
       payload = QByteArray::fromHex( parser.value("payload").toLocal8Bit());
       directDataSend = parser.isSet("direct-command");
       slaveId = parser.value("slave").toUInt();
       deviceType = parser.value("type");
//       mbusByteTimeoutMSec = parser.value("byte");
       update = parser.isSet("update");
       debug = parser.isSet("debug");
}
// execute what is commanded by the user
void MainController::executeArguments()
{
    // execut arguments
    if (!modbusInterface.isEmpty()){
        m_modbushandler = new ModbusHandler(this, modbusInterface, isDryRun);
        m_modbushandler->setBaudRate(baudRate);
//        m_modbushandler->setByteTimeout();
        if (!m_modbushandler->open()){
            fprintf(stderr, "Could not open modbus interface.\n");
            return;
        }
    } else {
        fprintf(stderr, "Please provide modbus interface\n");
        return;
    }

    if (deviceType == "OCU" || deviceType.isEmpty()){
        m_ocuHandler = new OpenFFUcontrolOCUhandler(this, m_modbushandler, isDryRun, debug);
        if (update){
            fprintf(stdout, " --- Starting OCU Update ---\n\n");
            if (pathToHexfile.isEmpty()){
                fprintf(stdout, "For update please provide a hex file.\n");
            } else {
                QByteArray program = getIntelHexContent(pathToHexfile);
                if (program == nullptr){
                    fprintf(stderr, "Hex file not readable.\n"
                                    "Update aborted.\n");
                    return;
                }
                if(m_ocuHandler->updateFirmware(slaveId, program)){
                    fprintf(stdout, "Update sucsessfull\n");
                } else{
                    fprintf(stdout, "Errors occured during update!\n"
                                    "Update might be written partialy, errors should be listed above.\n"
                                    "Rebooting might lead to you walking over to number %i with a programmer.\n", slaveId);
                    return;
                }
            }
        }
        if (functionCode != 0){
            fprintf(stdout, " --- Sending direct data ---\n\n");
            quint8 errorCode = 0;
            if (!payload.isEmpty()){
                errorCode = m_ocuHandler->sendRawCommand(slaveId, functionCode, payload);
            } else {
                errorCode = m_ocuHandler->sendRawCommand(slaveId, functionCode);
            }
            fprintf(stdout, "Direct data sent. Returend %s. Code %i\n", m_ocuHandler->errorString(errorCode).toLocal8Bit().data(), errorCode);
            return;
        }
    } else {
        fprintf(stderr, "Unknown device type %s\n", deviceType.toLocal8Bit().data());
        return;
    }}

QByteArray MainController::getIntelHexContent(QString file)
{
    IntelHexParser parser;
    if(!parser.parse(file)){
        return nullptr;
    }
    return parser.content();
}
