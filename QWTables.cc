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

#include "QWTables.h"
#include <QString>

const QWTables::OriginalChecksumTable QWTables::ourOriginalChecksumTable[] = {
  { "maps/start.bsp", 493454459 },
  { "maps/e1m1.bsp", 2902972546 },
  { "maps/e1m2.bsp", 1729102119 },
  { "maps/e1m3.bsp", 893792842 },
  { "maps/e1m4.bsp", 3990488693 },
  { "maps/e1m5.bsp", 2821463178 },
  { "maps/e1m6.bsp", 738207971 },
  { "maps/e1m7.bsp", 2547448602 },
  { "maps/e1m8.bsp", 79095617 },
  { "maps/e2m1.bsp", 3707072562 },
  { "maps/e2m2.bsp", 2945850701 },
  { "maps/e2m3.bsp", 4237894993 },
  { "maps/e2m4.bsp", 3273038793 },
  { "maps/e2m5.bsp", 3204615999 },
  { "maps/e2m6.bsp", 2443393921 },
  { "maps/e2m7.bsp", 2051006488 },
  { "maps/e3m1.bsp", 2427587873 },
  { "maps/e3m2.bsp", 2624353592 },
  { "maps/e3m3.bsp", 3285212440 },
  { "maps/e3m4.bsp", 2977500344 },
  { "maps/e3m5.bsp", 2440693297 },
  { "maps/e3m6.bsp", 767655416 },
  { "maps/e3m7.bsp", 272220593 },
  { "maps/e4m1.bsp", 3153093456 },
  { "maps/e4m2.bsp", 4294495000 },
  { "maps/e4m3.bsp", 1505685644 },
  { "maps/e4m4.bsp", 758847551 },
  { "maps/e4m5.bsp", 1771890676 },
  { "maps/e4m6.bsp", 102825880 },
  { "maps/e4m7.bsp", 2649489836 },
  { "maps/e4m8.bsp", 1018457175 },
  { "maps/end.bsp", 3151279269 },
  { "maps/dm1.bsp", 3318209203 },
  { "maps/dm2.bsp", 1710634548 },
  { "maps/dm3.bsp", 367136248 },
  { "maps/dm4.bsp", 2624578751 },
  { "maps/dm5.bsp", 2955757821 },
  { "maps/dm6.bsp", 1376311851 },
  { NULL, 0 }
};

quint32 QWTables::getOriginalMapChecksum(const QString &mapName)
{
  for(int i = 0; ; ++i)
  {
    if(!ourOriginalChecksumTable[i].mapname)
      return 0;

    if(mapName == QString(ourOriginalChecksumTable[i].mapname))
      return ourOriginalChecksumTable[i].checksum;
  }
  return 0;
}
