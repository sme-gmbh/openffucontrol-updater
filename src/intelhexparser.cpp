#include "intelhexparser.h"
#include "stdio.h"

IntelHexParser::IntelHexParser(QObject *parent) : QObject(parent)
{

}
// writes the program content of the hex file into the QByteArray program at the postions the address field of the hex fiel states
bool IntelHexParser::parse(QString file)
{
    fprintf(stdout, "Parsing hexfile\n");

    intelHexLine parsedLine;
    QString line;
    QByteArray buffer;

    quint16 extendetSegmentAddress = 0;

    quint64 extendedLinearAddress = 0;

    if(file.isEmpty())
        return false;

    QFile hexFile(file);

    if(!hexFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stdout, "Unable to open File: %s\n", hexFile.errorString().toLocal8Bit().append("\0").data());
        return false;
    }

    program.clear();
    program.fill(0xff, 256000);

    qint64 currentLine = 1;
    bool endOfFile = false;
    while (!endOfFile){

//        qDebug() << "InteHexParser parsing line: " << currentLine;
        buffer = hexFile.readLine();

        if( buffer.isEmpty()){
            break;
        }
        parsedLine = parseLine(buffer);

        if(!parsedLine.valid){  // abort parsing when a line gets marked as not valid
            fprintf(stdout, "line %lli not valid\n"
                            "aborting parse\n", currentLine);
            hexFile.close();
            return false;
        }

        switch (parsedLine.entyType) {
        case DATA_RECORD:
//            program.insert(extendedLinearAddress + parsedLine.address + extendetSegmentAddress * 16, parsedLine.data);
            quint32 adr;
            adr = (quint32)extendetSegmentAddress * 16 + (quint32)extendedLinearAddress + (quint32)parsedLine.address;
            for (quint8 i=0; i < parsedLine.bytecount; i++)
                program[adr + i] = parsedLine.data.at(i);

            break;
        case END_OF_FILE:
            endOfFile = true;
            break;
        case EXTENDET_SEGMENT_ADDRESS_RECORD:
            qDebug() << "Extendet Segment Address Record";
            extendetSegmentAddress = parsedLine.data.toShort();
            break;
        case START_SEGMENT_ADDRESS_RECORD:
            qDebug() << "Start Segment Address Record";
            return false;
            break;
        case EXTENDED_LINEAR_ADDRESS_RECORD:
            qDebug() << "Extendet Linear Address Record";
            extendedLinearAddress = (parsedLine.data.toLongLong() << 16);
            break;
        case START_LINEAR_ADDRESS_RECORD:
            qDebug() << "Start Linear Address Record";
            return false;
            break;
        default:
            fprintf(stdout, "unrecognised entry type at line %lli\n"
                            "aborting parse\n", currentLine);
            hexFile.close();
            return false;

        }

        currentLine++;
    }

    while ((quint8)program.back() == 0xff)
        program.chop(1);

    hexFile.close();

    fprintf(stdout, "Parsing hexfile done\n");
    return true;
}

QByteArray IntelHexParser::content()
{
    return program;
}

QByteArray IntelHexParser::content(QString file)
{
    if(parse(file)){
        return NULL;
    }
    return program;
}
// parses a intel hex file line into the intelHexLine struct and validates the checksum
IntelHexParser::intelHexLine IntelHexParser::parseLine(QString lineString)
{
    intelHexLine parsedLine;
    QByteArray lineByteArray;

    // lines must start with a colon
    if ((lineString.length() < 1) || (lineString[0] != ":")){
        fprintf(stdout, "IntelHexParser: parseLine failed: no valid start character\n");
        parsedLine.valid = false;
        return parsedLine;
    }

    lineString.remove(0, 1);

    lineByteArray = QByteArray::fromHex(lineString.toLocal8Bit());

    // line must not be empty
    if (lineByteArray.length() < 5){
        fprintf(stdout, "IntelHexParser: parseLine failed: line empty\n");
        parsedLine.valid = false;
        return parsedLine;
    }

    // read line content into struct
    parsedLine.bytecount = (quint8)lineByteArray.at(0);
    parsedLine.address = (quint8)lineByteArray.at(1) << 8;
    parsedLine.address |= (quint8)lineByteArray.at(2);
    parsedLine.entyType = (quint8)lineByteArray.at(3);
    parsedLine.data = lineByteArray.mid(4, 4 + parsedLine.bytecount);
    parsedLine.checksum = lineByteArray.back();

    // check if line fits with checksum
    quint64 sum = 0;
    for (qint32 i = 0; lineByteArray.length() > i; i++){
        sum = sum + (quint8)lineByteArray.at(i);
    }

    if ((sum & 0xFF) == 0){
        parsedLine.valid = true;
    }

    return parsedLine;
}
