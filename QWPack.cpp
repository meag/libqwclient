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

#include "QWPack.h"
#include <QStringList>

#define MAX_FILES_IN_PACK 2048

//on disk
typedef struct
{
	char	name[56];
	int		filePos, fileLen;
} dpackFile_t;

typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
} dpackHeader_t;

QWPack::QWPack()
{
}

bool QWPack::load(const QString& filename)
{
	QFile packFile(filename);
	if(!packFile.open(QIODevice::ReadOnly))
		return false;
	myFilename = filename;

	dpackHeader_t header;
	packFile.read((char*)&header, sizeof(dpackHeader_t));
	if(
	header.id[0] != 'P' ||
	header.id[1] != 'A' ||
	header.id[2] != 'C' ||
	header.id[3] != 'K'
	)
	{
		packFile.close();
		return false;
	}

	int fileCount;
	fileCount = header.dirlen / sizeof(dpackFile_t);
	if(fileCount > MAX_FILES_IN_PACK)
	{
		packFile.close();
		return false;
	}

	packFile.seek(header.dirofs);
	dpackFile_t *packData = new dpackFile_t[fileCount];
	packFile.read((char*)packData, header.dirlen);

	for(int i = 0; i < fileCount; ++i)
	{
		PackedFile file;

		file.name = packData[i].name;
		file.filePos = packData[i].filePos;
		file.fileLen = packData[i].fileLen;

		myFiles.push_back(file);
	}
	delete [] packData;
	packFile.close();

	return true;
}

bool QWPack::exists(const QString &filename) const
{
	QList<PackedFile>::const_iterator itr = myFiles.constBegin();

	while(itr != myFiles.constEnd())
	{
		const PackedFile* p = &(*itr);
		if(p->name == filename)
			return true;
		itr++;
	}
	return false;
}

QStringList QWPack::files() const
{
  QStringList list;

  QList<PackedFile>::const_iterator itr = myFiles.constBegin();

  while(itr != myFiles.constEnd())
  {
    const PackedFile* p = &(*itr);
    list.push_back(p->name);
    itr++;
  }
  return list;
}

bool QWPack::read(const QString &filename, char **fileData, quint64 *len) const
{
	QList<PackedFile>::const_iterator itr = myFiles.constBegin();

	while(itr != myFiles.constEnd())
	{
		const PackedFile* p = &(*itr);
		if(p->name == filename)
		{
			QFile packFile(myFilename);
			if(!packFile.open(QIODevice::ReadOnly))
				return false;

			packFile.seek(p->filePos);
			*fileData = new char[p->fileLen];
			packFile.read(*fileData, p->fileLen);
			*len = p->fileLen;
			return true;
		}
		itr++;
	}
	return false;
}


const QString& QWPack::fileName() const
{
	return myFilename;
}
