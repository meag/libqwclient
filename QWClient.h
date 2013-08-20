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

#ifndef QWCLIENT_H
#define QWCLIENT_H

#include "qwclient_global.h"

class QWCLIENTSHARED_EXPORT QWClient {
	friend class QWClientPrivate;
public:
  enum ClientState { DisconnectedState, ConnectingState, ConnectedState, LastState };

	QWClient();
	virtual ~QWClient();

  void connect(const char* host, quint16 port);
  void run();
  void disconnect();
	void observe();
	void join();
	void setQuakeFolder(const char* path);
	void setName(const char* name);
	void setColor(quint8 bottom, quint8 top);
	void setSpectator(bool spectate = true);
	void setPing(quint16 ping);
	void setRate(quint16 rate);
  void setPassword(const char* password);
	void sendCmd(const char* cmd);
  const QString& gameDir() const;
  const QString& quakeDir() const;
	void reconnect();
	const char* host() const;
	quint16 port() const;
  ClientState state() const;

	static void stripColor(char* string);

protected:
	/* Overridable functions */
	virtual void onChallenge();
	virtual void onConnection();
	virtual void onConnected();
	virtual void onModelListFile(const char* fileName);
	virtual void onSoundListFile(const char* fileName);
	virtual void onDownloadStarted(const char* fileName);
	virtual void onDownloadFinished();
	virtual void onDownloadProgress(int progress);
	virtual void onDisconnect(); //disconnected by the remote side
	virtual void onPrint(int level, const char* msg);
	virtual void onCenterPrint(const char* msg);
	virtual void onStuffedCmd(const char* cmd); //some of these commands are already handled to estabilish connection
	virtual void onDamage(int armor, int blood);
	virtual void onLevelChanged(int playerNum, const char* levelName, float gravity, float stopSpeed, float maxSpeed, float spectatorMaxSpeed, float accelerate, float airAccelerate, float waterAccelerate, float friction, float waterFriction, float entGravity);
	virtual void onPlaySound(int soundNum);
	virtual void onUpdateFrags(int playerNum, int frags);
	virtual void onUpdatePing(int playerNum, int ping);
	virtual void onUpdatePL(int playerNum, int pl);
	virtual void onUpdateUserInfo(int playerNum, int userID, const char* userInfoString);
	virtual void onSetInfo(int playerNum, const char* key, const char* value);
	virtual void onServerInfo(const char* key, const char* value);
	virtual void onMaxSpeedChange(float maxSpeed);
	virtual void onEntGravityChange(float entGravity);
	virtual void onSetPause(bool paused);
	virtual void onPlayerInfo(int playerNum, float x, float y, float z);
	virtual void onError(const char* description);
	virtual void onOOBPrint(const char* msg);
	virtual void onOOBCommand(const char* command);
	virtual void onOOBEcho(const char* msg);

private:
	class QWClientPrivate* const myImplementation;
};

#endif // QWCLIENT_H
