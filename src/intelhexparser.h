#ifndef INTELHEXPARSER_H
#define INTELHEXPARSER_H

#include <QByteArray>
#include <QFile>
#include <QObject>

#include <QDebug>

class IntelHexParser : public QObject
{
    Q_OBJECT
public:
    explicit IntelHexParser(QObject *parent = nullptr);

    bool parse(QString file);
    QByteArray content();
    QByteArray content(QString file);

private:
    QByteArray program;

    typedef enum {
        DATA_RECORD                     = 00,
        END_OF_FILE                     = 01,
        EXTENDET_SEGMENT_ADDRESS_RECORD  = 02,
        START_SEGMENT_ADDRESS_RECORD    = 03,
        EXTENDED_LINEAR_ADDRESS_RECORD   = 04,
        START_LINEAR_ADDRESS_RECORD      = 05
    } entyType;

    struct intelHexLine{
        quint8 bytecount = 0;
        quint16 address = 0;
        quint8 entyType = 6;
        QByteArray data;
        quint8 checksum = 0;
        bool valid = false;
    };

    intelHexLine parseLine(QString line);

signals:

public slots:

};

#endif // INTELHEXPARSER_H
