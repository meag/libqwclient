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
#include <stdio.h>

QWClient::QWClient():
  myImplementation(new QWClientPrivate(this))
{
}

void QWClient::connect(const char *host, quint16 port)
{
	myImplementation->connect(host, port);
}

void QWClient::run()
{
	myImplementation->run();
}

void QWClient::disconnect()
{
  myImplementation->disconnect();
}

QWClient::~QWClient()
{
	delete myImplementation;
}

void QWClient::join()
{
	myImplementation->join();
}

void QWClient::observe()
{
	myImplementation->observe();
}

void QWClient::setQuakeFolder(const char *path)
{
	myImplementation->setQuakeFolder(path);
}

void QWClient::setColor(quint8 bottom, quint8 top)
{
	myImplementation->setColor(bottom, top);
}

void QWClient::setName(const char *name)
{
	myImplementation->setName(name);
}

void QWClient::setSpectator(bool spectate)
{
	myImplementation->setSpectator(spectate);
}

void QWClient::stripColor(char *string)
{
	QWClientPrivate::stripColor(string);
}

void QWClient::sendCmd(const char *cmd)
{
	QString command(cmd);
	myImplementation->sendCmd(cmd);
}

void QWClient::setRate(quint16 rate)
{
	myImplementation->setRate(rate);
}

void QWClient::setPing(quint16 ping)
{
	myImplementation->setPing(ping);
}

void QWClient::reconnect()
{
	myImplementation->reconnect();
}

const char* QWClient::host() const
{
  static char host[255];
  memcpy(host, myImplementation->host().toAscii().data(), myImplementation->host().toAscii().size()+1);
  return host;
}

quint16 QWClient::port() const
{
	return myImplementation->port();
}

QWClient::ClientState QWClient::state() const
{
	return myImplementation->state();
}

const QString& QWClient::gameDir() const
{
  return myImplementation->gameDir();
}

const QString& QWClient::quakeDir() const
{
  return myImplementation->quakeDir();
}

void QWClient::setPassword(const char *password)
{
  myImplementation->setPassword(QString(password));
}

//========================================================================
// Events

void QWClient::onCenterPrint(const char *)
{

}

void QWClient::onDamage(int, int)
{

}

void QWClient::onDisconnect()
{

}

void QWClient::onDownloadFinished()
{

}

void QWClient::onDownloadProgress(int)
{

}

void QWClient::onDownloadStarted(const char *)
{

}

void QWClient::onEntGravityChange(float)
{

}

void QWClient::onLevelChanged(int, const char*, float, float, float, float, float, float, float, float, float, float)
{

}

void QWClient::onMaxSpeedChange(float)
{

}

void QWClient::onModelListFile(const char *)
{

}

void QWClient::onPlayerInfo(int, float, float, float)
{

}

void QWClient::onPlaySound(int)
{

}

void QWClient::onPrint(int, const char *)
{

}

void QWClient::onServerInfo(const char *, const char *)
{

}

void QWClient::onSetInfo(int, const char *, const char *)
{

}

void QWClient::onSetPause(bool)
{

}

void QWClient::onSoundListFile(const char *)
{

}

void QWClient::onStuffedCmd(const char *)
{

}

void QWClient::onUpdateFrags(int, int)
{

}

void QWClient::onUpdatePing(int, int)
{

}

void QWClient::onUpdatePL(int, int)
{

}

void QWClient::onUpdateUserInfo(int, int, const char *)
{

}

void QWClient::onError(const char *)
{

}

void QWClient::onOOBCommand(const char *)
{

}

void QWClient::onOOBEcho(const char *)
{

}

void QWClient::onOOBPrint(const char *)
{

}

void QWClient::onChallenge()
{

}

void QWClient::onConnection()
{

}

void QWClient::onConnected()
{

}
