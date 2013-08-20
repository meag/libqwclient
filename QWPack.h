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

#ifndef QWPACK_H
#define QWPACK_H

#include <QString>
#include <QFile>

class QWPack
{
public:
	QWPack();
	bool	load(const QString &filename);
	bool	exists(const QString &filename) const;
	bool	read(const QString &filename, char **read, quint64 *len) const;
	const QString& fileName() const;
  QStringList files() const;

private:
	typedef struct
	{
		QString	name;
		int	filePos, fileLen;
	} PackedFile;

	QString						myFilename;
	QList<PackedFile> myFiles;
};

#endif // QWPACK_H
