#include "intelhexparser.h"
#include "stdio.h"

IntelHexParser::IntelHexParser(QObject *parent) : QObject(parent)
{

}

bool IntelHexParser::parse(QString file)
{
    fprintf(stdout, "Parsing hexfile\n");
    qDebug() << "IntelHexParser start parse";
    intelHexLine parsedLine;
    QString line;
    QByteArray buffer;

    if(filePath.isEmpty())
        return 0;

    QFile hexFile(file);

    if(!hexFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stdout, "Unable to open File: %s\n", hexFile.errorString().toLocal8Bit().append("\0").data());
    }

    qint64 currentLine = 1;
    while (true){

        qDebug() << "InteHexParser parsing line: " << currentLine;
        buffer = hexFile.readLine();

        if( buffer.isEmpty()){
            break;
        }
        parsedLine = parseLine(buffer);

        if(!parsedLine.valid){  // abort parsing when a line gets marked as not valid
            fprintf(stdout, "line %ll not valid\n", &currentLine);
            hexFile.close();
            return 1;
        }

        currentLine++;
    }

    hexFile.close();
    return 0;
}
// parses a intel hex file line into the intelHexLine struct and validates the checksum
IntelHexParser::intelHexLine IntelHexParser::parseLine(QString lineString)
{
    intelHexLine parsedLine;
    QByteArray lineByteArray;

    qDebug() << "IntelHexParser parseLine";

    // lines must start with a colon
    if (lineString[0] != ":"){
        fprintf(stdout, "IntelHexParser parseLine failed,\n"
                        "no valid start character\n");
        parsedLine.valid = false;
        return parsedLine;
    }

    lineString.remove(0, 1);

    lineByteArray = QByteArray::fromHex(lineString.toLocal8Bit());

    // line must not be empty
    if (lineByteArray.isEmpty()){
        fprintf(stdout, "IntelHexParser parseLine failed,\n"
                        "line empty\n");
        parsedLine.valid = false;
        return parsedLine;
    }

    // read line content into struct
    parsedLine.bytecount = lineByteArray.at(0);
    parsedLine.address = lineByteArray.mid(1,2).toShort();
    parsedLine.entyType = lineByteArray.at(3);
    parsedLine.data = lineByteArray.mid(4, 4 + parsedLine.bytecount/2);
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
