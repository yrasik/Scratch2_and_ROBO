/*         "Scratch2 to ROBO translator"
Copyright (C) 2016 Yuri Stepanenko stepanenkoyra@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tcp_sockets.h"


TcpSockets::TcpSockets(QObject *parent) :
    QObject(parent)
{

}


TcpSockets::~TcpSockets()
{

}


void TcpSockets::incoming(qintptr ID)
{
  QTcpSocket* socket = new QTcpSocket();

  while( SClients.isEmpty() == false )
  {
    SClients[ SClients.lastKey() ]->close();
    SClients.remove( SClients.lastKey() );
  }

  if(!socket->setSocketDescriptor(ID))
  {
    emit error(socket->error());
        qDebug() << "error";
    return;
  }

  SClients[ID]=socket;
  connect(SClients[ID],SIGNAL(readyRead()),this, SLOT(slotReadClient()));

}


void TcpSockets::data_from_main_app(const Message &s)
{
  if( SClients.isEmpty() == false )
  {
    SClients[ SClients.firstKey() ]->write(s.Data);
    SClients[ SClients.firstKey() ]->close();
    SClients.remove( SClients.firstKey() );
  }
}


void TcpSockets::slotReadClient()
{
  QTcpSocket* clientSocket = (QTcpSocket*)sender();
  qintptr idusersocs = clientSocket->socketDescriptor();

  QByteArray arr = clientSocket->readAll();
 // emit this->message_to_gui(TEXT_MESSAGE_TYPE__GREEN, arr);
  emit this->data_to_main_app( Message::init(idusersocs, arr, QTime::currentTime() ) );
}



