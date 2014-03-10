# _libqwclient_
## Compiling
> $ qmake
> $ make

## Installing
> \# make install

## Usage
1. Link libqwclient.so to your project;
2. `#include <qwclient.h>`;
3. Read tru the QWClient class to see what virtual methods can be redefined to create your own bot;
4. Don't forget to call the run() method of QWClient in order to keep the client running.

## Example
For a live sample check out the QuakeWorld Bot project at https://gitlab.netdome.biz/community-messaging-project/qwbot.