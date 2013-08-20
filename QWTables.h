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

#ifndef QWTABLES_H
#define QWTABLES_H

#include <QtGlobal>

class QWTables
{
public:
  /**
    Returns the checksum for an original ID1 map.

    @param  mapName The absolute map name path
    @return The checksum if found otherwise 0
  */
  static quint32 getOriginalMapChecksum(const QString& mapName);

  private:
  ///////////////////////////////////
  // Original ID1 Map checksums
  ///////////////////////////////////
  struct OriginalChecksumTable
  {
    const char *mapname;
    quint32 checksum;
  };
  static const OriginalChecksumTable ourOriginalChecksumTable[];
  QWTables();
};

#endif // QWTABLES_H
