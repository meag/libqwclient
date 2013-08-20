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

#ifndef QWCLIENTPRIVATE_H
#define QWCLIENTPRIVATE_H

#include <qglobal.h>
#include <QBuffer>
#include <QDataStream>
#include <QHostAddress>
#include <QList>
#include "QWClient.h"
#include "quakedef.h"

class QWClient;
class QWPack;
class QWClientPrivate
{
public:
	QWClientPrivate(QWClient *client);
	~QWClientPrivate();

	void							connect(const char *host, quint16 port);
	void							run();
  void							disconnect();
	void							observe();
	void							join();
  void              setBindHost(const QString& host);
	void							setName(const char *name);
	void							setTeam(const char *team);
	void							setColor(quint8 bottom, quint8 top);
	void							setQuakeFolder(const QString& path);
	void							setSpectator(bool spectate = true);
  void              setPassword(const QString& password);
	void							setPing(quint16 ping);
	void							setRate(quint16 rate);
	void							sendCmd(const QString& cmd);
  const QString&    gameDir() const;
  const QString&    quakeDir() const;
	void							reconnect();
	const QString			host() const { return myHost.toString(); }
	quint16						port() const { return myPort; }
	QWClient::ClientState state() const { return myState; }
	static void				stripColor(char* string);

private:
	class QWClient*		myClient;
	class QUdpSocket* mySocket;
	class QTime*			myTime;
	class QTime*			myLastServerReplyTime;
	class QFile*			myDownload; //current active download

	QHostAddress			myHost;
	quint16						myPort;

	/* Incoming data buffers */
	QByteArray				myInData;
	QBuffer						myInBuffer;
	QDataStream				myInStream;

	/* Outgoing client data buffers */
	QDataStream				myUnreliableOutStream;
	QBuffer						myUnreliableOutBuffer;
	QByteArray				myUnreliableOutData;

	QDataStream				myReliableOutStream;
	QBuffer						myReliableOutBuffer;
	QByteArray				myReliableOutData;
	QByteArray				myReliableData;		//saves reliable messages not acked

	/* Everything is assembled here b4 sending */
	QDataStream				myOutStream;
	QBuffer						myOutBuffer;
	QByteArray				myOutData;

	bool							myBadReadFlag;

	static const char*ClientName;
	static const char*ClientVersion;
	QString						myClientName;
	QString						myClientVersion;

	QWClient::ClientState	myState;
	quint16						myQPort;
	quint32						myProtocolVersion;
	quint32						myFTEProtocolExtensions;
	quint32						myServerCount;
  QString						myGameDir;
	QString						myQuakeDir;
	QString						myMapName;
  QString           myPassword; // For connecting on servers that require a password

	quint32						myIncomingSeq;
	quint32						myIncomingAck;
	quint32						myOutgoingSeq;
	quint32						myLastRealiableSeq;
	bool							myIncomingSeqReliableFlag;
	bool							myIncomingAckReliableFlag;
	bool							myOutgoingSeqReliableFlag;
	quint8						myPacketLoss;
	quint16						myPing;

	/* Download */
	QList<QWPack*>		myPacks;

	/* Client Cvars */
	quint16						myRate;
	quint8						myTopColor;
	quint8						myBottomColor;
	QString						myName;
	bool							mySpectatorFlag;
	QString						myTeam;

	/* NameFun Conversion */
	static char				ourReadableCharsTable[256];
	static bool				ourReadableCharsTableInitialized;
	static void				fillReadableCharsTable();

	void							reloadPackFiles();
	void							loadPackFile(const QString& filename);

	void							sendConnectionless(const QByteArray& data);
	void							sendMovement(); //required on MVDSV
  void							sendToServer(bool dontWait = false);
	void							readPackets();

	void							startDownload(const QString& filename);

	bool							fileExists(const QString& filename);
	bool							readFile(const QString& filename, char **data, quint64 *len);

	void							preSpawn(int mapChecksum);

	static unsigned		blockCheckSum(void *buffer, int len);
	quint32						mapChecksum(const QString& mapName);

	//========================================================================
	// Parsing functions
	/* Main parsing functions */
	void							parseServerMessage();
	void							parseConnectionless();

	/* Helpers */
	static float			littleFloat(float f);
	static quint16		littleShort(quint16 s);
	static quint32		littleLong(quint32 l);

	/* Reading */
	bool							checkForBadRead(quint8 typeSize);
	float							readCoord();
	float							readAngle();
	float							readAngle16();
	quint8						readByte();
	float							readFloat();
	qint16						readShort();
	qint32						readLong();
	const QString			readString();
	void							readUserDeltaCmd(userCmd_t *from, userCmd_t *move);
	void							parseDelta(entityState_t *from, entityState_t *to, int bits);

	/* Writing */
	static void				writeByte(QDataStream* stream, const quint8 b);
	static void				writeShort(QDataStream* stream, const quint16 s);
	static void				writeLong(QDataStream* stream, const quint32 l);
	static void				writeString(QDataStream* stream, const QString& str);


	/* Command parsers */
  void							parseSvcModellist();//s
  void							parseSvcFTEModellistShort();//s
  void							parseSvcDisconnect();//
  void							parseSvcNoop();//
  void							parseSvcPrint();//
  void							parseSvcCenterPrint();//
  void							parseSvcStuffText();//
  void							parseSvcDamage();//
  void							parseSvcServerData();//s
  void							parseSvcSetAngle();//
  void							parseSvcLightStyle();//
  void							parseSvcSound();//
  void							parseSvcStopSound();//
  void							parseSvcUpdateFrags();//
  void							parseSvcUpdatePing();//
  void							parseSvcUpdatePL();//
  void							parseSvcUpdateEnterTime();//
  void							parseSvcSpawnBaseLine();//
  void							parseSvcSpawnStatic();//
  void							parseSvcTempEntity();//
  void							parseSvcKilledMonster();//
  void							parseSvcFoundSecret();//
  void							parseSvcUpdateStat();//
  void							parseSvcUpdateStatLong();//
  void							parseSvcSpawnStaticSound();//
  void							parseSvcCDTrack();//
  void							parseSvcIntermission();//
  void							parseSvcFinale();//
  void							parseSvcSellScreen();//
  void							parseSvcSmallKick();//
  void							parseSvcBigKick();//
  void							parseSvcMuzzleFlash();//
  void							parseSvcUpdateUserinfo();//
  void							parseSvcSetinfo();//
  void							parseSvcServerinfo();//
  void							parseSvcDownload();//
  void							parseChunkedDownload();//NA
  void							parseSvcPlayerinfo();//
  void							parseSvcNails();//fixed
  void							parseSvcChokeCount();//
  void							parseSvcSoundlist();//
  void							parseSvcPacketEntities();//look at it
  void							parseSvcDeltaPacketEntities();//look at it
  void							parseSvcMaxSpeed();//
  void							parseSvcEntGravity();//
  void							parseSvcSetPause();//
  void							parseSvcNails2();//fixed
  void							parseSvcFTESpawnBaseline2();//
  void							parseSvcQizmoVoice();//
  void							parseSvcFTEVoiceChat();//hmmf
  void							parseSvcFTESpawnStatic2();//FIXED readbyte supposed to be readShort.
  void              parseSvcUpdateName();
  void              parseSvcUpdateColors();
  void              parseSvcSignonNum();
  void              parseSvcParticle();
  void              parseSvcVersion();
  void              parseSvcClientData();
  void              parseSvcTime();
};

#endif // QWCLIENTPRIVATE_H
