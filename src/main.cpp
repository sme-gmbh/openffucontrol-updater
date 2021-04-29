#include <QCoreApplication>
#include "maincontroller.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName("openffucontrol");
    QCoreApplication::setOrganizationDomain("sme-gmbh.com");
    QCoreApplication::setApplicationName("openFFUcontrol-updater");
//    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
    MainController c(nullptr, QCoreApplication::arguments());

    //return a.exec();
}
