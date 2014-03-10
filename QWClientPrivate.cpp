/*
GNU General Public License version 3 notice

Copyright (C) 2012 Mihawk <luiz@netdome.biz>. All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see < http://www.gnu.org/licenses/ >.
*/

#include "QWClient.h"
#include "QWClientPrivate.h"
#include "QWPack.h"
#include "QWTables.h"
#include <QUdpSocket>
#include <QTime>
#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QFileInfoList>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QtEndian>
#include <QDebug>

const char* QWClientPrivate::ClientName		= "libqwclient";
const char* QWClientPrivate::ClientVersion = "0.1";

QWClientPrivate::QWClientPrivate(QWClient* client):
    myClient(client),
    mySocket(new QUdpSocket),
    myTime(new QTime),
    myLastServerReplyTime(new QTime),
    myDownload(new QFile),
    myClientName(ClientName),
    myClientVersion(ClientVersion),
    myState(QWClient::DisconnectedState),
    myGameDir("qw"),
    myQuakeDir(QCoreApplication::applicationDirPath()),
    myPing(666),
    myRate(3000),
    myTopColor(0),
    myBottomColor(0),
    myName(ClientName),
    mySpectatorFlag(true),
    myTeam("lqwc")
{
    /* Setup IO streams */
    myInBuffer.setBuffer(&myInData);
    myInStream.setDevice(&myInBuffer);
    myInStream.setByteOrder(QDataStream::LittleEndian);
    myInBuffer.open(QIODevice::ReadOnly);

    myUnreliableOutBuffer.setBuffer(&myUnreliableOutData);
    myUnreliableOutStream.setDevice(&myUnreliableOutBuffer);
    myUnreliableOutBuffer.open(QIODevice::WriteOnly);
    myUnreliableOutStream.setByteOrder(QDataStream::LittleEndian);

    myReliableOutBuffer.setBuffer(&myReliableOutData);
    myReliableOutStream.setDevice(&myReliableOutBuffer);
    myReliableOutBuffer.open(QIODevice::WriteOnly);
    myReliableOutStream.setByteOrder(QDataStream::LittleEndian);

    myOutBuffer.setBuffer(&myOutData);
    myOutStream.setDevice(&myOutBuffer);
    myOutBuffer.open(QIODevice::ReadWrite);
    myOutStream.setByteOrder(QDataStream::LittleEndian);

    reloadPackFiles();
}

void QWClientPrivate::setPing(quint16 ping)
{
    myPing = qBound<quint16>(13, ping, 999);
}

void QWClientPrivate::reconnect()
{
    disconnect();
    connect(myHost.toString().toLatin1().data(), myPort);
}

void QWClientPrivate::setRate(quint16 rate)
{
    myRate = qBound<quint16>(2500, rate, 30000);
    if(myState != QWClient::ConnectedState)
        return;

    sendCmd("setinfo \"rate\" \"" + QString::number(rate) + "\"");
}

void QWClientPrivate::setSpectator(bool spectate)
{
    if(myState == QWClient::ConnectedState)
    {
        if(spectate && !mySpectatorFlag)
            observe();
        else if(!spectate && mySpectatorFlag)
            join();

        return;
    }

    mySpectatorFlag = spectate;
}

void QWClientPrivate::setPassword(const QString &password)
{
    myPassword = password;
}

void QWClientPrivate::setQuakeFolder(const QString &path)
{
    myQuakeDir = path;
    QDir dir(myQuakeDir);
    dir.mkpath(myGameDir);
    reloadPackFiles();
}

void QWClientPrivate::join()
{
    mySpectatorFlag = false;

    if(myState != QWClient::ConnectedState)
        return;

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo \"spectator\" \"\"");
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "join");
}

void QWClientPrivate::observe()
{
    mySpectatorFlag = true;

    if(myState != QWClient::ConnectedState)
        return;

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo \"spectator\" \"" + QString::number(mySpectatorFlag) + "\"");
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "observe");
}

const QString& QWClientPrivate::gameDir() const
{
    return myGameDir;
}

const QString& QWClientPrivate::quakeDir() const
{
    return myQuakeDir;
}

void QWClientPrivate::setColor(quint8 bottom, quint8 top)
{
    myBottomColor = bottom;
    myTopColor = top;

    if(myState != QWClient::ConnectedState)
        return;

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo \"bottomcolor\" \"" + QString::number(myBottomColor) + "\"");
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo \"topcolor\" \"" + QString::number(myTopColor) + "\"");
}

void QWClientPrivate::setName(const char *name)
{
    myName = QString(name);

    if(myState == QWClient::ConnectedState)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, "setinfo \"name\" \"" + myName + "\"");
    }
}

void QWClientPrivate::setTeam(const char *team)
{
    myTeam = QString(team);

    if(myState == QWClient::ConnectedState)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, "setinfo \"name\" \"" + myTeam + "\"");
    }
}

void QWClientPrivate::sendCmd(const QString &cmd)
{
    if(myState != QWClient::ConnectedState)
        return;

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, cmd);
}

QWClientPrivate::~QWClientPrivate()
{
    delete myTime;
    delete myLastServerReplyTime;
    delete mySocket;
    delete myDownload;
}

static QRegExp packRegex("pak[0-9]+\\.pak", Qt::CaseInsensitive);

void QWClientPrivate::reloadPackFiles()
{
    for(int i = 0; i < myPacks.size(); ++i)
        delete myPacks.at(i);
    myPacks.clear();

    QDir gameDir(myQuakeDir + "/" + myGameDir);
    if(!gameDir.isReadable())
        return;
    QFileInfoList files = gameDir.entryInfoList(QStringList("*.pak"), QDir::Files);
    for(int i = 0; i < files.size(); ++i)
    {
        if(packRegex.indexIn(files.at(i).fileName()) != -1)
        {
            QWPack* pack = new QWPack();
            if(!pack->load(files.at(i).absoluteFilePath()))
            {
                delete pack;
                continue;
            }
            myPacks.push_back(pack);
        }
    }
}

void QWClientPrivate::run()
{
    /* Read and parse packets sent by the server */
    readPackets();

    /* Send something to the server to keep the connection alive */
    sendToServer();
}

void QWClientPrivate::sendToServer(bool dontWait)
{
    /* Check for resend */
    if(myState == QWClient::ConnectingState)
    {
        if(myTime->elapsed() >= 5000)
        {
            sendConnectionless("getchallenge\n");
            myClient->onChallenge();
            myTime->restart();
        }
        return;
    }

    /* Go full speed when connecting or downloading a new map */
    if(!dontWait && (myState != QWClient::ConnectedState || myDownload->isOpen() ? 12 : myPing) > myTime->elapsed())
        return;

    sendMovement(); //send some movement every frame

    bool sendReliable = false;

    if(myIncomingAck > myLastRealiableSeq && myIncomingAckReliableFlag != myOutgoingSeqReliableFlag)
        sendReliable = true;

    if(!myReliableData.size() && myReliableOutData.size())
    {
        myReliableData = myReliableOutData;
        myOutgoingSeqReliableFlag ^= 1;
        sendReliable = true;

        myReliableOutData.clear();
        myReliableOutStream.device()->seek(0);
    }

    /* Write packet header */
    myOutData.clear();
    myOutStream.device()->seek(0);

    myOutStream << (myOutgoingSeq | (sendReliable << 31));
    myOutStream << (myIncomingSeq | (myIncomingSeqReliableFlag << 31));
    myOutStream << myQPort;
    myOutgoingSeq++;

    /* Write reliable buffer first */
    if(sendReliable)
    {
        myOutData.append(myReliableData);
        myLastRealiableSeq = myOutgoingSeq;
    }

    /* Unreliable part afterwards */
    if(myOutData.size() + myUnreliableOutData.size() < MAX_MSGLEN && myUnreliableOutData.size())
    {
        myOutData.append(myUnreliableOutData);
        myUnreliableOutData.clear();
        myUnreliableOutStream.device()->seek(0);
    }

    /* Finally send the packet */
    mySocket->write(myOutData);
    mySocket->waitForBytesWritten();
    myOutData.clear();
    myOutStream.device()->seek(0);

    myTime->restart();
}

void QWClientPrivate::readPackets()
{
    if(!mySocket->isOpen())
        return;

    if(!mySocket->hasPendingDatagrams())
    {
        if(myLastServerReplyTime->secsTo(QTime::currentTime()) >= 30)
        {
            myClient->onError("Client Timed Out.");
            disconnect();
            return;
        }
        return;
    }

    *myLastServerReplyTime = QTime::currentTime();

    myInData.resize(mySocket->pendingDatagramSize());
    //	myInData = mySocket->readAll();
    mySocket->readDatagram(myInData.data(), mySocket->pendingDatagramSize());

    myInStream.device()->seek(0);

    quint32 seq;
    myInStream >> seq;

    if(seq == 0xffffffff)
        parseConnectionless();
    else
        parseServerMessage();
}

void QWClientPrivate::parseConnectionless()
{
    quint8 c;

    c = readByte();
    switch(c)
    {
    case S2C_CHALLENGE:
    {
        QString challenge;
        QString connString;

        challenge = readString();

        connString.append("connect " + QString::number(PROTOCOL_VERSION) + " " + QString::number(myQPort) + " " + challenge);
        connString.append(" \"\\rate\\" + QString::number(myRate));
        if(!myPassword.isEmpty())
            connString.append("\\password\\" + myPassword);
        connString.append("\\msg\\1\\noaim\\1\\topcolor\\" + QString::number(myTopColor) + "\\bottomcolor\\" + QString::number(myBottomColor) + "\\w_switch\\2\\b_switch\\2\\*client\\" + myClientName);
        connString.append(" " + myClientVersion + "\\name\\" + myName + "\\team\\" + myTeam + "\\spectator\\" + (mySpectatorFlag ? "1" : "0") + "\\pmodel\\33168\\emodel\\6967\\*z_ext\\383\"");
        myInStream.device()->seek(0);
        sendConnectionless(connString.toLatin1());
        myClient->onConnection();
    }
        break;

    case S2C_CONNECTION:
    {
        myState = QWClient::ConnectedState;
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, "new");
        myClient->onConnected();
    }
        break;

    case A2C_PRINT:
        myClient->onOOBPrint(readString().toLatin1().data());
        break;

    case A2C_CLIENT_COMMAND:
        myClient->onOOBCommand(readString().toLatin1().data());
        break;

    case A2A_PING:
        sendConnectionless(QString(QChar(A2A_ACK)).toLatin1());
        break;

    case A2A_ACK:
        //			qDebug("Ack");
        break;

    case A2A_NACK:
        //			qDebug("Nack");
        break;

    case A2A_ECHO:
        myClient->onOOBEcho(readString().toLatin1().data());
        break;
    }
}

bool QWClientPrivate::fileExists(const QString &filename)
{
    /* Search on the gamedir first */
    if(QFile::exists(myQuakeDir + "/" + myGameDir + "/" + filename))
        return true;

    /* Search quake pak files next */
    for(int i = 0; i < myPacks.size(); ++i)
    {
        if(myPacks[i]->exists(filename))
            return true;
    }

    return false;
}

bool QWClientPrivate::readFile(const QString &filename, char **data, quint64 *len)
{
    /* Try disk */
    QFile file(myQuakeDir + "/" + myGameDir + "/" + filename);
    *len = 0;
    if(file.open(QIODevice::ReadOnly))
    {
        *len = file.size();
        *data = new char[*len];
        file.read(*data, *len);
        return true;
    }

    /* Try packs */
    for(int i = 0; i < myPacks.size(); ++i)
    {
        QWPack* pack = myPacks.at(i);

        if(!pack->exists(filename))
            continue;
        pack->read(filename, data, len);
        return true;
    }

    return false;
}

//int	bitCounts[32];	/// just for protocol profiling
void QWClientPrivate::parseDelta(entityState_t *from, entityState_t *to, int bits)
{
    int			i;
    int			morebits;

    // set everything to the state we are delta'ing from
    *to = *from;

    to->number = bits & 511;
    bits &= ~511;

    if(bits & U_MOREBITS)
    {	// read in the low order bits
        i = readByte();
        bits |= i;
    }

    if(bits & U_FTE_EVENMORE && myFTEProtocolExtensions) {
        morebits = readByte();
        if (morebits & U_FTE_YETMORE)
            morebits |= readByte() << 8;
    } else {
        morebits = 0;
    }
    //	// count the bits for net profiling
    //	for (i=0 ; i<16 ; i++)
    //		if (bits&(1<<i))
    //			bitCounts[i]++;

    to->flags = bits;

    if(bits & U_MODEL)
        to->modelindex = readByte();

    if(bits & U_FRAME)
        to->frame = readByte();

    if(bits & U_COLORMAP)
        to->colormap = readByte();

    if(bits & U_SKIN)
        to->skinnum = readByte();

    if(bits & U_EFFECTS)
        to->effects = readByte();

    if(bits & U_ORIGIN1)
        to->origin[0] = readCoord();

    if(bits & U_ANGLE1)
        to->angles[0] = readAngle();

    if(bits & U_ORIGIN2)
        to->origin[1] = readCoord();

    if(bits & U_ANGLE2)
        to->angles[1] = readAngle();

    if(bits & U_ORIGIN3)
        to->origin[2] = readCoord();

    if(bits & U_ANGLE3)
        to->angles[2] = readAngle();

    if(bits & U_SOLID)
    {}

    if(morebits & U_FTE_TRANS && myFTEProtocolExtensions & FTE_PEXT_TRANS)
        readByte();
    if(morebits & U_FTE_ENTITYDBL)
    {}
    if(morebits & U_FTE_ENTITYDBL2)
    {}
    if(morebits & U_FTE_MODELDBL)
    {}
}

//========================================================================
// Parser

quint32 QWClientPrivate::littleLong(quint32 l)
{
    if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        return qToLittleEndian<quint32>(l);
    else
        return l;
}

quint16 QWClientPrivate::littleShort(quint16 s)
{
    if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        return qToLittleEndian<quint16>(s);
    else
        return s;
}

float QWClientPrivate::littleFloat(float f)
{
    if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        return qToLittleEndian<float>(f);
    else
        return f;
}

bool QWClientPrivate::checkForBadRead(quint8 typeSize)
{
    if(myInStream.device()->pos()+typeSize > myInStream.device()->size())
    {
        myBadReadFlag = true;
        return true;
    }
    return false;
}

float QWClientPrivate::readAngle()
{
    if(checkForBadRead(1))
        return -1;

    quint8 angle;

    myInStream >> angle;
    return angle * (360.0f/256);
}

float QWClientPrivate::readAngle16()
{
    if(checkForBadRead(2))
        return -1;

    quint16 angle;

    myInStream >> angle;
    return angle * (360.0f/65536);
}

float QWClientPrivate::readCoord()
{
    if(checkForBadRead(2))
        return -1;

    quint16 coord;

    myInStream >> coord;
    return coord * (1.0f/8);
}

quint8 QWClientPrivate::readByte()
{
    if(checkForBadRead(1))
        return 0xff;

    quint8 byte;

    myInStream >> byte;
    return byte;
}

float QWClientPrivate::readFloat()
{
    union
    {
        quint8	b[4];
        int			l;
        float		f;
    } d;

    d.b[0] = readByte();
    d.b[1] = readByte();
    d.b[2] = readByte();
    d.b[3] = readByte();

    return littleFloat(d.f);
}

const QString QWClientPrivate::readString()
{
    QString str;
    quint8 c;
    c = readByte();
    while(c && c != 255) //bad read or null terminator
    {
        str.append(c);
        c = readByte();
    }
    return str;
}

qint32 QWClientPrivate::readLong()
{
    if(checkForBadRead(4))
        return -1;

    quint32 l;

    myInStream >> l;
    return l;
}

qint16 QWClientPrivate::readShort()
{
    if(checkForBadRead(2))
        return -1;

    qint16 s;

    myInStream >> s;
    return s;
}

//========================================================================

void QWClientPrivate::parseSvcNoop()
{

}

void QWClientPrivate::parseSvcDisconnect()
{
    mySocket->close();
    myState = QWClient::DisconnectedState;
    myClient->onDisconnect();
}

void QWClientPrivate::parseSvcPrint()
{
    quint8	level = readByte();
    QString msg		= readString();
    myClient->onPrint(level, msg.toLatin1().data());
}

void QWClientPrivate::parseSvcCenterPrint()
{
    myClient->onCenterPrint(readString().toLatin1().data());
}

void QWClientPrivate::parseSvcStuffText()
{
    QStringList commands = readString().split("\n");

    for(int i = 0; i < commands.size(); ++i)
    {
        QString cmd = commands.at(i);

        if(cmd == "reconnect" || cmd == "cmd new")
        {
            writeByte(&myReliableOutStream, clc_stringcmd);
            writeString(&myReliableOutStream, "new");
        }
        else if(cmd == "cmd pext")
        {
            writeByte(&myReliableOutStream, clc_stringcmd);
            writeString(&myReliableOutStream, "pext 0x00000000 0x00000000 0x00000000");
        }
        else if(cmd.startsWith("cmd spawn"))
        {
            writeByte(&myReliableOutStream, clc_stringcmd);
            writeString(&myReliableOutStream, cmd.section(' ', 1));
        }
        else if(cmd.startsWith("cmd prespawn"))
        {
            writeByte(&myReliableOutStream, clc_stringcmd);
            writeString(&myReliableOutStream, cmd.section(' ', 1));
        }
        else if(cmd == "skins")
        {
            writeByte(&myReliableOutStream, clc_stringcmd);
            writeString(&myReliableOutStream, QString("begin " + QString::number(myServerCount)));
        }
        else if(cmd.startsWith("packet"))
        {
            QRegExp regex("\"(.+)\"");
            int pos = regex.indexIn(cmd);
            if(pos != -1)
                sendConnectionless(regex.capturedTexts().at(1).toLatin1());
        }
        myClient->onStuffedCmd(cmd.toLatin1().data());
    }
}

void QWClientPrivate::parseSvcDamage()
{
    int armor = readByte();
    int blood = readByte();
    myClient->onDamage(armor, blood);
    readCoord();
    readCoord();
    readCoord();
}

void QWClientPrivate::parseSvcServerData()
{
    while (1) {
        myProtocolVersion = readLong();
        if(myProtocolVersion == PROTOCOL_VERSION_FTE || myProtocolVersion == PROTOCOL_VERSION_FTE2)
        {
            myFTEProtocolExtensions = readLong();
            continue;
        }
        if(myProtocolVersion == PROTOCOL_VERSION)
            break;
    }

    myServerCount = readLong();

    myGameDir = readString();
    QDir quakeDir(myQuakeDir);
    if(!quakeDir.cd(myGameDir))
        quakeDir.mkdir(myGameDir);

    quint8 playerNum = readByte();		//playernum
    if(playerNum & 0x80)
    {
        mySpectatorFlag = true;
        playerNum &= ~0x80;
    }
    else
        mySpectatorFlag = false;

    QString lvlName = readString();
    float a = readFloat();
    float b = readFloat();
    float c = readFloat();
    float d = readFloat();
    float e = readFloat();
    float f = readFloat();
    float g = readFloat();
    float h = readFloat();
    float i = readFloat();
    float j = readFloat();
    myClient->onLevelChanged(
                playerNum,
                lvlName.toLatin1().data(),
                a,b,c,d,e,f,g,h,i,j
                );
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, QString("soundlist " + QString::number(myServerCount) + " 0"));

    if(myDownload->isOpen())
        myDownload->close();
}

void QWClientPrivate::parseSvcSetAngle()
{
    readAngle();
    readAngle();
    readAngle();
}

void QWClientPrivate::parseSvcLightStyle()
{
    readByte();
    readString();
}

void QWClientPrivate::parseSvcSound()
{
    quint16 channel;

    channel = readShort();
    if (channel & SND_VOLUME)
        readByte();
    if (channel & SND_ATTENUATION)
        readByte();
    myClient->onPlaySound(readByte());
    readCoord();
    readCoord();
    readCoord();
}

void QWClientPrivate::parseSvcStopSound()
{
    readShort();
}

void QWClientPrivate::parseSvcUpdateFrags()
{
    //printf("svc_updatefrags\n");
    quint8 playerNum = readByte();
    quint16 frags = readShort();
    myClient->onUpdateFrags(playerNum, frags);
}

void QWClientPrivate::parseSvcUpdatePing()
{
    quint8 playerNum = readByte();
    quint16 ping = readShort();
    myClient->onUpdatePing(playerNum, ping);
}

void QWClientPrivate::parseSvcUpdatePL()
{
    quint8 playerNum = readByte();
    quint8 pl = readByte();
    myClient->onUpdatePL(playerNum, pl);
}

void QWClientPrivate::parseSvcUpdateEnterTime()
{
    readByte();
    readFloat();
}

void QWClientPrivate::parseSvcSpawnBaseLine()
{
    //printf("svc_spawnbaseline\n");
    readShort();
    readByte();
    readByte();
    readByte();
    readByte();
    for(int i = 0; i < 3; i++)
    {
        readCoord();
        readAngle();
    }
}

void QWClientPrivate::parseSvcSpawnStatic()
{
    //printf("svc_spawnstatic\n");
    //rawReadShort();
    readByte();
    readByte();
    readByte();
    readByte();
    for(int i = 0; i < 3; i++)
    {
        readCoord();
        readAngle();
    }
}

void QWClientPrivate::parseSvcTempEntity()
{
    //printf("svc_temp_entity\n");
    quint8 c = readByte();
    bool	 parsed = false;

    switch(c)
    {
    case TE_LIGHTNING1:
    case TE_LIGHTNING2:
    case TE_LIGHTNING3:
        readShort();

        readCoord();
        readCoord();
        readCoord();

        readCoord();
        readCoord();
        readCoord();
        parsed = true;
        break;

    case TE_GUNSHOT:
        readByte();
        readCoord();
        readCoord();
        readCoord();
        parsed = true;
        break;

    case TE_BLOOD:
        readByte();
        readCoord();
        readCoord();
        readCoord();
        parsed = true;
        break;

    case TE_LIGHTNINGBLOOD:
        readCoord();
        readCoord();
        readCoord();
        parsed = true;
        break;
    }

    if(!parsed)
    {
        readCoord();
        readCoord();
        readCoord();
    }
}

void QWClientPrivate::parseSvcUpdateStat()
{
    readByte();
    readByte();
}

void QWClientPrivate::parseSvcUpdateStatLong()
{
    //printf("svc_updatestatlong\n");
    readByte();
    readLong();
}

void QWClientPrivate::parseSvcSpawnStaticSound()
{
    //printf("svc_spawnstaticsound\n");
    readCoord();
    readCoord();
    readCoord();
    readByte();
    readByte();
    readByte();
}

void QWClientPrivate::parseSvcCDTrack()
{
    readByte();
}

void QWClientPrivate::parseSvcIntermission()
{
    //printf("svc_intermission\n");
    readCoord();
    readCoord();
    readCoord();
    readAngle();
    readAngle();
    readAngle();
}

void QWClientPrivate::parseSvcFinale()
{
    //printf("svc_finale\n");
    readString();
}

void QWClientPrivate::parseSvcSellScreen()
{

}

void QWClientPrivate::parseSvcSmallKick()
{

}

void QWClientPrivate::parseSvcBigKick()
{

}

void QWClientPrivate::parseSvcMuzzleFlash()
{
    //printf("svc_muzzleflash\n");
    readShort();
}

void QWClientPrivate::parseSvcUpdateUserinfo()
{
    quint8 playerNum = readByte();
    quint32 userID = readLong();
    QString info = readString();
    myClient->onUpdateUserInfo(playerNum, userID, info.toLatin1().data());
}

void QWClientPrivate::parseSvcSetinfo()
{
    //printf("svc_setinfo\n");
    QString key, value;
    int playerNum = readByte();
    key = readString();
    value = readString();

    //that must be done
    if(key == "rate")
    {
        myRate = value.toUInt();
        sendCmd("setinfo \"rate\" \"" + value + "\"");
    }

    myClient->onSetInfo(playerNum, key.toLatin1().data(), value.toLatin1().data());
}

void QWClientPrivate::parseSvcServerinfo()
{
    //printf("svc_serverinfo\n");
    QString key, value;

    key = readString();
    value = readString();
    myClient->onServerInfo(key.toLatin1().data(), value.toLatin1().data());
}

void QWClientPrivate::startDownload(const QString &filename)
{
    if(myDownload->isOpen())
        myDownload->close();

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, QString("download " + filename));

    if(filename.contains('/'))
    {
        QString path = filename.mid(0, (filename.size() - filename.section('/', -1).size() - 1));
        QDir quakeDir(myGameDir);
        quakeDir.mkpath(myQuakeDir + "/" + myGameDir + "/" + path);
    }

    //generate disk path
    int tmpNo = 0;
    QString tmpFileName = myQuakeDir + "/" + myGameDir + "/" + filename + QString::number(tmpNo) + ".tmp";
    while(myDownload->exists(tmpFileName))
    {
        tmpNo++;
        tmpFileName = myQuakeDir + "/" + myGameDir + "/" + filename + QString::number(tmpNo) + ".tmp";
    }

    myDownload->setFileName(tmpFileName);
    myDownload->open(QIODevice::WriteOnly);

    myClient->onDownloadStarted(filename.toLatin1().data());
}

#if 0
void QWClientPrivate::parseChunkedDownload()
{
    QString	svname;
    int			totalsize;
    int			chunknum;
    char		data[DLBLOCKSIZE];

    chunknum = readLong();
    if(chunknum < 0)
    {
        totalsize = readLong();
        svname    = readString();

        if(myDownload->isOpen())
        {
            // Ensure FILE is closed
            if(totalsize != -3) // -3 = dl stopped, so this known issue, do not warn
                qDebug("cls.download shouldn't have been set\n");
            myDownload->close();
        }

        if(totalsize < 0)
        {
            switch(totalsize)
            {
            case -3: qDebug("Server cancel downloading file %s\n", svname.toLatin1().data());			break;
            case -2: qDebug("Server permissions deny downloading file %s\n", svname.toLatin1().data());	break;
            default: qDebug("Couldn't find file %s on the server\n", svname.toLatin1().data());			break;
            }
            myDownload->close();
            return;
        }

        if(cls.downloadmethod == DL_QWCHUNKS)
        {
            qDebug("Received second download - \"%s\"\n", svname.toLatin1().data());
            disconnect();
        }

        if(!myDownload->isOpen())
        {
            qDebug("Failed to open %s", myDownload->fileName());
            return;
        }

        cls.downloadmethod  = DL_QWCHUNKS;
        cls.downloadpercent = 0;

        chunked_download_number++;

        downloadsize        = totalsize;

        firstblock    = 0;
        receivedbytes = 0;
        blockcycle    = -1;	//so it requests 0 first. :)
        memset(recievedblock, 0, sizeof(recievedblock));
        return;
    }

    MSG_ReadData(data, DLBLOCKSIZE);

    if (myDownload->isOpen())
        return;

    if (cls.downloadmethod != DL_QWCHUNKS)
        Host_Error("cls.downloadmethod != DL_QWCHUNKS\n");

    if(chunknum < firstblock)
        return;

    if(chunknum - firstblock >= MAXBLOCKS)
        return;

    if(recievedblock[chunknum&(MAXBLOCKS-1)])
        return;

    receivedbytes += DLBLOCKSIZE;
    recievedblock[chunknum&(MAXBLOCKS-1)] = true;

    while(recievedblock[firstblock&(MAXBLOCKS-1)])
    {
        recievedblock[firstblock&(MAXBLOCKS-1)] = false;
        firstblock++;
    }

    fseek(cls.download, chunknum * DLBLOCKSIZE, SEEK_SET);
    if (downloadsize - chunknum * DLBLOCKSIZE < DLBLOCKSIZE)	//final block is actually meant to be smaller than we recieve.
        fwrite(data, 1, downloadsize - chunknum * DLBLOCKSIZE, cls.download);
    else
        fwrite(data, 1, DLBLOCKSIZE, cls.download);

    cls.downloadpercent = receivedbytes/(float)downloadsize*100;

    tm = Sys_DoubleTime() - cls.downloadstarttime; // how long we dl-ing
    cls.downloadrate = (tm ? receivedbytes / 1024 / tm : 0); // some average dl speed in KB/s
}
#endif

void QWClientPrivate::parseSvcDownload()
{
    //	if(myFTEProtocolExtensions & FTE_PEXT_CHUNKEDDOWNLOADS)
    //	{
    //		parseChunkedDownload();
    //		return;
    //	}

    qint16 size;
    quint8 percent;

    size = readShort();
    percent = readByte();

    if(size == -1)
    {
        //file not found
        disconnect();
        return;
    }

    if(!myDownload->isOpen())
    {
        myInStream.device()->seek(myInStream.device()->pos() + size);
        return;
    }

    char*	temp = new char[size];
    myInStream.readRawData(temp, size);
    myDownload->write(temp, size);
    myDownload->waitForBytesWritten(1000);
    delete[] temp;

    myClient->onDownloadProgress(percent);
    if(percent != 100)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, "nextdl");
    }
    else
    {
        myDownload->close();
        QString normalFileName = myDownload->fileName().left(myDownload->fileName().length()-5); //strip #.tmp
        if(!myDownload->rename(normalFileName))
        {
            QFile::remove(normalFileName);
            myDownload->rename(normalFileName); //now it should succeed
        }

        myClient->onDownloadFinished();

        preSpawn(mapChecksum(myMapName));
    }
}

void QWClientPrivate::readUserDeltaCmd(userCmd_t *from, userCmd_t *move)
{
    quint8 bits;

    memcpy(move, from, sizeof(*move));

    bits = readByte();

    // read current angles
    if(bits & CM_ANGLE1)
        move->angles[0] = readAngle16();
    if(bits & CM_ANGLE2)
        move->angles[1] = readAngle16();
    if(bits & CM_ANGLE3)
        move->angles[2] = readAngle16();

    // read movement
    if(bits & CM_FORWARD)
        move->forwardmove = readShort();
    if(bits & CM_SIDE)
        move->sidemove = readShort();
    if(bits & CM_UP)
        move->upmove = readShort();

    // read buttons
    if(bits & CM_BUTTONS)
        move->buttons = readByte();

    if(bits & CM_IMPULSE)
        move->impulse = readByte();

    // read time to run command
    move->msec = readByte();

}

void QWClientPrivate::parseSvcPlayerinfo()
{
    quint16		flags;
    userCmd_t from, move;

    memset(&from, 0, sizeof(userCmd_t));
    memset(&move, 0, sizeof(userCmd_t));
    int playerNum = readByte();
    flags = readShort();
    float x = readCoord();
    float y = readCoord();
    float z = readCoord();

    myClient->onPlayerInfo(playerNum, x, y, z);

    readByte();

    if(flags & PF_MSEC)
        readByte();
    if(flags & PF_COMMAND)
        readUserDeltaCmd(&from, &move);

    for(int i = 0; i < 3; i++)
    {
        if(flags & (PF_VELOCITY1<<i))
            readShort();
    }
    if(flags & PF_MODEL)
        readByte();
    if(flags & PF_SKINNUM)
        readByte();
    if(flags & PF_EFFECTS)
        readByte();
    if(flags & PF_WEAPONFRAME)
        readByte();
    if(flags & PF_TRANS_Z && myFTEProtocolExtensions & FTE_PEXT_TRANS)
        readByte();
}

void QWClientPrivate::parseSvcNails()
{
    //printf("svc_nails\n");
    quint8 c = readByte();
    for(int i = 0; i < c; i++)
    {
        for(int j = 0; j < 6; j++)
            readByte();
    }
}

void QWClientPrivate::parseSvcChokeCount()
{
    readByte();
}

void QWClientPrivate::parseSvcModellist()
{
    quint8 i = readByte();
    bool	 firstLoop = true;
    for(;;)
    {
        QString s = readString();
        if(s.isEmpty())
            break;
        if(!i && firstLoop)
        {
            myMapName = s;
            firstLoop = false;
        }
        myClient->onModelListFile(s.toLatin1().data());
    }
    i = readByte();
    if(i)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, QString("modellist " + QString::number(myServerCount) + " " + QString::number(i)));
        return;
    }

    if(!fileExists(myMapName) && !QWTables::getOriginalMapChecksum(myMapName))
    {
        startDownload(myMapName);
        return;
    }
    preSpawn(mapChecksum(myMapName));
}

void QWClientPrivate::parseSvcSoundlist()
{
    quint8 i;

    i = readByte();
    for(;;)
    {
        QString s = readString();
        if(s.isEmpty())
            break;
        myClient->onSoundListFile(s.toLatin1().data());
    }

    i = readByte();
    if(i)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, QString("soundlist " + QString::number(myServerCount) + " " + QString::number(i)));
        return;
    }

    //continue login
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, QString("modellist " + QString::number(myServerCount) + " 0"));
}

void QWClientPrivate::parseSvcPacketEntities()
{
    int	word;
    entityState_t	olde, newe;

    memset(&olde, 0, sizeof(olde));

    while (1)
    {
        word = (unsigned short)readShort();

        if (!word)
            break;	// done

        parseDelta(&olde, &newe, word);
    }
}

void QWClientPrivate::parseSvcDeltaPacketEntities()
{
    parseSvcPacketEntities();
}

void QWClientPrivate::parseSvcMaxSpeed()
{
    myClient->onMaxSpeedChange(readFloat());
}

void QWClientPrivate::parseSvcEntGravity()
{
    myClient->onEntGravityChange(readFloat());
}

void QWClientPrivate::parseSvcSetPause()
{
    myClient->onSetPause(readByte());
}

void QWClientPrivate::parseSvcNails2()
{
    quint8 c = readByte();
    for(int i = 0; i < c; i++)
    {
        readByte();
        for(int j = 0; j < 6; j++)
            readByte();
    }
}

void QWClientPrivate::parseSvcFTEModellistShort()
{
    quint16 i = readShort();
    bool	  firstLoop = true;
    for(;;)
    {
        QString s;
        s = readString();
        if(s.isEmpty())
            break;
        if(!i && firstLoop)
        {
            myMapName = s;
            firstLoop = false;
        }
        myClient->onModelListFile(s.toLatin1().data());
    }
    i = readByte();
    if(i)
    {
        writeByte(&myReliableOutStream, clc_stringcmd);
        writeString(&myReliableOutStream, QString("modellist " + QString::number(myServerCount) + " " + QString::number(i)));
        return;
    }

    if(!fileExists(myMapName) && !QWTables::getOriginalMapChecksum(myMapName))
    {
        startDownload(myMapName);
        return;
    }
    preSpawn(mapChecksum(myMapName));
}

void QWClientPrivate::parseSvcFTESpawnBaseline2()
{
    entityState_t nullst, es;

    if(!(myFTEProtocolExtensions & FTE_PEXT_SPAWNSTATIC2))
    {
        myClient->onError("illegible server message\nsvc_fte_spawnbaseline2 without FTE_PEXT_SPAWNSTATIC2\n");
        disconnect();
        return;
    }

    memset(&nullst, 0, sizeof (entityState_t));
    memset(&es, 0, sizeof (entityState_t));

    parseDelta(&nullst, &es, readShort());
}

void QWClientPrivate::parseSvcQizmoVoice()
{
    // Read the two-byte header.
    readByte();
    readByte();

    // 32 bytes of voice data follow
    for(int i = 0; i < 32; i++)
        readByte();
}

void QWClientPrivate::parseSvcFTEVoiceChat()
{
    readByte();
    readByte();
    readByte();
    quint16 i = readShort();
    myInStream.device()->seek(myInStream.device()->pos() + i);
}

void QWClientPrivate::parseSvcFTESpawnStatic2()
{
    if(myFTEProtocolExtensions & FTE_PEXT_SPAWNSTATIC2)
    {
        entityState_t from, to;
        memset(&from, 0, sizeof(entityState_t));
        memset(&to, 0, sizeof(entityState_t));
        parseDelta(&from, &to, readShort());
    }
}

void QWClientPrivate::parseSvcKilledMonster()
{

}

void QWClientPrivate::parseSvcFoundSecret()
{

}

void QWClientPrivate::parseSvcUpdateName()
{
    readByte();
    readString();
}

void QWClientPrivate::parseSvcUpdateColors()
{
    readByte();
    readByte();
}

void QWClientPrivate::parseSvcTime()
{
    readFloat();
}

void QWClientPrivate::parseSvcSignonNum()
{
    readByte();
}

void QWClientPrivate::parseSvcParticle()
{
    for(int i = 0; i < 3; ++i)
        readCoord();
    for(int i = 0; i < 3; ++i)
        readByte();
    readByte();
    readByte();
}

void QWClientPrivate::parseSvcClientData()
{
    qDebug() << "WHY?!?!?!?!?!?! WHY ME BEING CALLED!";
}

void QWClientPrivate::parseSvcVersion()
{

}

void QWClientPrivate::preSpawn(int mapChecksum)
{
    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo pmodel 33168");

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, "setinfo emodel 6967");

    writeByte(&myReliableOutStream, clc_stringcmd);
    writeString(&myReliableOutStream, QString("prespawn " + QString::number(myServerCount) + " 0 " + QString::number(mapChecksum)));
}

void QWClientPrivate::parseServerMessage()
{
    quint32 incomingSeq, incomingAck;
    bool		incomingSeqReliable, incomingAckReliable;

    myBadReadFlag = false;

    myInStream.device()->seek(0);
    myInStream >> incomingSeq;
    myInStream >> incomingAck;
    incomingSeqReliable = incomingSeq >> 31;
    incomingAckReliable = incomingAck >> 31;
    incomingSeq &= ~0x80000000;
    incomingAck &= ~0x80000000;

    if(incomingSeq <= myIncomingSeq)
        return;

    myPacketLoss = incomingSeq - (myIncomingSeq + 1);

    if(incomingAckReliable == myOutgoingSeqReliableFlag)
        myReliableData.clear();

    if(incomingSeqReliable)
        myIncomingSeqReliableFlag ^= 1;

    myIncomingSeq = incomingSeq;
    myIncomingAck = incomingAck;
    myIncomingAckReliableFlag = incomingAckReliable;

    while(!myInStream.atEnd())
    {
        if(myBadReadFlag)
        {
            myClient->onError("Bad read from server.");
            disconnect();
            return;
        }

        quint8 c;
        quint8 last = 0;

        myInStream >> c;
        if(c == 0xff)
            break;

        switch(c)
        {
        case svc_bad:
            myClient->onError("Bad read from server.");
            disconnect();
            return;

        case svc_nop:
            parseSvcNoop();
            break;

        case svc_disconnect:
            parseSvcDisconnect();
            break;

        case svc_print:
            parseSvcPrint();
            break;

        case svc_centerprint:
            parseSvcCenterPrint();
            break;

        case svc_stufftext:
            parseSvcStuffText();
            break;

        case svc_damage:
            parseSvcDamage();
            break;

        case svc_serverdata:
            parseSvcServerData();
            break;

        case svc_setangle:
            parseSvcSetAngle();
            break;

        case svc_lightstyle:
            parseSvcLightStyle();
            break;

        case svc_sound:
            parseSvcSound();
            break;

        case svc_stopsound:
            parseSvcStopSound();
            break;

        case svc_updatefrags:
            parseSvcUpdateFrags();
            break;

        case svc_updateping:
            parseSvcUpdatePing();
            break;

        case svc_updatepl:
            parseSvcUpdatePL();
            break;

        case svc_updateentertime:
            parseSvcUpdateEnterTime();
            break;

        case svc_spawnbaseline:
            parseSvcSpawnBaseLine();
            break;

        case svc_spawnstatic:
            parseSvcSpawnStatic();
            break;

        case svc_temp_entity:
            parseSvcTempEntity();
            break;

        case svc_killedmonster:
            parseSvcKilledMonster();
            break;

        case svc_foundsecret:
            parseSvcFoundSecret();
            break;

        case svc_updatestat:
            parseSvcUpdateStat();
            break;

        case svc_updatestatlong:
            parseSvcUpdateStatLong();
            break;

        case svc_spawnstaticsound:
            parseSvcSpawnStaticSound();
            break;

        case svc_cdtrack:
            parseSvcCDTrack();
            break;

        case svc_intermission:
            parseSvcIntermission();
            break;

        case svc_finale:
            parseSvcFinale();
            break;

        case svc_sellscreen:
            parseSvcSellScreen();
            break;

        case svc_smallkick:
            parseSvcSmallKick();
            break;

        case svc_bigkick:
            parseSvcBigKick();
            break;

        case svc_muzzleflash:
            parseSvcMuzzleFlash();
            break;

        case svc_updateuserinfo:
            parseSvcUpdateUserinfo();
            break;

        case svc_setinfo:
            parseSvcSetinfo();
            break;

        case svc_serverinfo:
            parseSvcServerinfo();
            break;

        case svc_download:
            parseSvcDownload();
            break;

        case svc_playerinfo:
            parseSvcPlayerinfo();
            break;

        case svc_nails:
            parseSvcNails();
            break;

        case svc_chokecount:
            parseSvcChokeCount();
            break;

        case svc_modellist:
            parseSvcModellist();
            break;

        case svc_soundlist:
            parseSvcSoundlist();
            break;

        case svc_packetentities:
            parseSvcPacketEntities();
            break;

        case svc_deltapacketentities:
            parseSvcDeltaPacketEntities();
            break;

        case svc_maxspeed:
            parseSvcMaxSpeed();
            break;

        case svc_entgravity:
            parseSvcEntGravity();
            break;

        case svc_setpause:
            parseSvcSetPause();
            break;

        case svc_nails2:
            parseSvcNails2();
            break;

        case svc_fte_modellistshort:
            parseSvcFTEModellistShort();
            break;

        case svc_fte_spawnbaseline2:
            parseSvcFTESpawnBaseline2();
            break;

        case svc_qizmovoice:
            parseSvcQizmoVoice();
            break;

        case svc_fte_voicechat:
            parseSvcFTEVoiceChat();
            break;

        case svc_fte_spawnstatic2:
            parseSvcFTESpawnStatic2();
            break;
        case nq_svc_time:
            parseSvcTime();
            break;
        case nq_svc_clientdata:
            parseSvcClientData();
            break;
        case nq_svc_version:
            parseSvcVersion();
            break;
        case nq_svc_particle:
            parseSvcParticle();
            break;
        case nq_svc_signonnum:
            parseSvcSignonNum();
            break;
        case nq_svc_updatecolors:
            parseSvcUpdateColors();
            break;
        case nq_svc_updatename:
            parseSvcUpdateName();
            break;
        default:
            myClient->onError(QString("Unknown message from server. Last Cmd: [" + QString::number(last) + "] Current Cmd: [" + QString::number(c) + "]").toLatin1().data());
            disconnect();
            return;
        }
        last = c;
    }
}

void QWClientPrivate::connect(const char *host, quint16 port)
{
    if(myState != QWClient::DisconnectedState)
        return;

    /* Disabled this, blocking too much, now user is supposed to send the string already resolved. */
    //	QHostInfo hi = QHostInfo::fromName(host);
    //	if(hi.error() != QHostInfo::NoError)
    //	{
    //		myClient->onError(hi.errorString().toLatin1().data());
    //		return;
    //	}

    myHost.setAddress(host);
    myPort = port;

    myIncomingSeq = 0;
    myIncomingAck = 0;
    myOutgoingSeq = 0;
    myLastRealiableSeq = 0;
    myIncomingSeqReliableFlag = false;
    myIncomingAckReliableFlag = false;
    myOutgoingSeqReliableFlag = false;
    myPacketLoss = 0;

    mySocket->connectToHost(myHost, myPort);
    mySocket->waitForConnected();
    sendConnectionless("getchallenge\n");
    myClient->onChallenge();
    myQPort = qrand() & 0xffff;
    myTime->start();

    myState = QWClient::ConnectingState;

    *myLastServerReplyTime = QTime::currentTime();
}

void QWClientPrivate::setBindHost(const QString &host)
{
    QHostAddress address(host);
    mySocket->bind(address, 0);
}

void QWClientPrivate::disconnect()
{
    if(myState == QWClient::ConnectedState)
    {
        writeByte(&myUnreliableOutStream, clc_stringcmd);
        writeString(&myUnreliableOutStream, "drop");
        sendToServer(true);
        writeByte(&myUnreliableOutStream, clc_stringcmd);
        writeString(&myUnreliableOutStream, "drop");
        sendToServer(true);
        writeByte(&myUnreliableOutStream, clc_stringcmd);
        writeString(&myUnreliableOutStream, "drop");
        sendToServer(true);
    }
    mySocket->close();
    myState = QWClient::DisconnectedState;
}

void QWClientPrivate::sendConnectionless(const QByteArray &data)
{
    QByteArray d;
    d.append("\xff\xff\xff\xff");
    d.append(data);
    mySocket->write(d);
    mySocket->waitForBytesWritten();
}

void QWClientPrivate::sendMovement()
{
    myUnreliableOutStream << (quint8)clc_move << (quint8)0x10 << (quint8)myPacketLoss << (quint8)0x00 << (quint8)0x00 << (quint8)0x00 << (quint8)0x22 << (quint8)0x00 << (quint8)0x21;
}

//=====================================================================

// "GPL map" support.  If we encounter a map with a known "GPL" CRC,
// we fake the CRC so that, on the client side, the CRC of the original
// map is transferred to the server, and on the server side, comparison
// of clients' CRC is done against the orignal one
typedef struct {
    const char *mapname;
    unsigned int original;
    unsigned int gpl;
} csentry_t;

static csentry_t table[] = {
    // CRCs for AquaShark's "simpletextures" maps
    { "dm1", 0xc5c7dab3, 0x7d37618e },
    { "dm2", 0x65f63634, 0x7b337440 },
    { "dm3", 0x15e20df8, 0x912781ae },
    { "dm4", 0x9c6fe4bf, 0xc374df89 },
    { "dm5", 0xb02d48fd, 0x77ca7ce5 },
    { "dm6", 0x5208da2b, 0x200c8b5d },
    { "end", 0xbbd4b4a5, 0xf89b12ae }, // this is the version with the extra room
    { NULL, 0, 0 },
};

int Com_TranslateMapChecksum (const char *mapname, unsigned int checksum)
{
    csentry_t *p;

    //	Com_Printf ("Map checksum (%s): 0x%x\n", mapname, checksum);

    for (p = table; p->mapname; p++)
        if (!strcmp(p->mapname, mapname)) {
            if (checksum == p->gpl)
                return p->original;
            else
                return checksum;
        }

    return checksum;
}

quint32 QWClientPrivate::mapChecksum(const QString &mapName)
{
    // Check if this is an original map, if it is we have the checksum table ready
    quint32     checksum = 0;
    checksum = QWTables::getOriginalMapChecksum(mapName);
    if(checksum)
        return checksum;

    char*		mapdata;
    quint64 maplen;

    if(!readFile(mapName, &mapdata, &maplen))
        return 0;
    if(!maplen || !mapdata)
        return 0;

    dheader_t* header;
    uchar*		 mod_base;

    header = (dheader_t*)mapdata;
    mod_base = (uchar*)mapdata;

    for(int i = 0; i < HEADER_LUMPS; ++i)
    {
        if(i == LUMP_ENTITIES || i == LUMP_VISIBILITY || i == LUMP_LEAFS || i == LUMP_NODES)
            continue;
        checksum ^= blockCheckSum(mod_base + header->lumps[i].fileofs, header->lumps[i].filelen);
    }
    delete[] mapdata;

    QString	cleanMapName;
    cleanMapName = mapName.section('/', -1);
    cleanMapName.chop(4); //strip .bsp

    return Com_TranslateMapChecksum(cleanMapName.toLatin1().data(), checksum);
}

unsigned QWClientPrivate::blockCheckSum(void* buffer, int len)
{
    QByteArray digest = QCryptographicHash::hash(QByteArray((const char*)buffer, len), QCryptographicHash::Md4);
    int *d = (int*)digest.data();
    return (d[0] ^ d[1] ^ d[2] ^ d[3]);
}

void QWClientPrivate::writeByte(QDataStream *stream, const quint8 b)
{
    *stream << b;
}

void QWClientPrivate::writeLong(QDataStream *stream, const quint32 l)
{
    *stream << l;
}

void QWClientPrivate::writeShort(QDataStream *stream, const quint16 s)
{
    *stream << s;
}

void QWClientPrivate::writeString(QDataStream *stream, const QString &str)
{
    stream->writeRawData(str.toLatin1().data(), str.size()+1);
}

//========================================================================
// NAME FUN

char QWClientPrivate::ourReadableCharsTable[256] = {	'.', '_' , '_' , '_' , '_' , '.' , '_' , '_' , '_' , '_' , '\n' , '_' , '\n' , '>' , '.' , '.',
                                                        '[', ']', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '_', '_', '_'
                                                   };
bool QWClientPrivate::ourReadableCharsTableInitialized = false;

void QWClientPrivate::fillReadableCharsTable()
{
    int i;

    for(i = 32; i < 127; i++)
        ourReadableCharsTable[i] = ourReadableCharsTable[128 + i] = i;
    ourReadableCharsTable[127] = ourReadableCharsTable[128 + 127] = '_';

    for(i = 0; i < 32; i++)
        ourReadableCharsTable[128 + i] = ourReadableCharsTable[i];
    ourReadableCharsTable[128] = '_';
    ourReadableCharsTable[10 + 128] = '_';
    ourReadableCharsTable[12 + 128] = '_';

    ourReadableCharsTableInitialized = true;
}

void QWClientPrivate::stripColor(char* string)
{
    /* Parse simple quake red text and special chars */
    if(!ourReadableCharsTableInitialized)
        fillReadableCharsTable();

    while(*string) {
        *string = ourReadableCharsTable[(unsigned char)*string] & 127;
        string++;
    }
}
