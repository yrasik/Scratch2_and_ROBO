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

#include "robo_tcp_server.h"
#include "tcp_sockets.h"

RoboTcpServer::RoboTcpServer(QObject *parent) :
    QTcpServer(parent)
{

  connect(&tcp_s_thread, &TcpSockets::data_to_main_app, this, &RoboTcpServer::data_from_http);

  connect(&serial, &SerialThread::response, this, &RoboTcpServer::showResponse);
  connect(&serial, &SerialThread::error, this, &RoboTcpServer::processError);
  connect(&serial, &SerialThread::timeout, this, &RoboTcpServer::processTimeout);

  connect(&serial, &SerialThread::message_to_gui, this, &RoboTcpServer::message_from_serial);

  connect(&pars, &parser::message_to_gui, this, &RoboTcpServer::message_from_parser);

  connect(&tcp_s_thread, &TcpSockets::message_to_gui, this, &RoboTcpServer::message_from_parser);



}

RoboTcpServer::~RoboTcpServer()
{

}


void RoboTcpServer::message_from_parser(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes)
{
  emit this->message_to_gui(mes_type, mes);
}


void RoboTcpServer::message_from_serial(unsigned int mes_type, const QString &mes)
{
  emit this->message_to_gui( (enum TEXT_MESSAGE_TYPE)(mes_type), mes);
}


void RoboTcpServer::showResponse(const QByteArray &s)
{
  if( pars.rx_serial_to_web_check(s) == 0 ) //Пакеты ком порт местами меняет иногда..
  {
    tcp_s_thread.data_from_main_app( pars.rx_serial_to_web(s) );
  }

  emit slot_showResponse(s);
}


void RoboTcpServer::processError(const QString &s)
{
  qDebug() << "ERROR 20: Dialog::processError";

  emit slot_processError();
}


void RoboTcpServer::processTimeout(const QString &s)
{
  qDebug() << "ERROR 21: Dialog::processTimeout";
  emit slot_processTimeout();
}


signed char RoboTcpServer::startServer(const QString &name_serial_port, int serial_timeout)
{
    this->name_serial_port = name_serial_port;
    this->serial_timeout = serial_timeout;


    int port = 50208;

    if( ! this->listen(QHostAddress::LocalHost, port) )
    {
      QString  err = "ERROR 92: Не могу запустить сервер\n";
      emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
      qDebug() << err;
      return -92;
    }
    else
    {
      QString  err = "Сервер запущен. Прослушивается порт " + QString::number(port) + "...\n";
      emit this->message_to_gui(TEXT_MESSAGE_TYPE__GOOD, err);
      qDebug() << err;
      return 0;
    }



}



void RoboTcpServer::stopServer(void)
{
  this->close();

  QString  err = "Сервер остановлен. \n";
  emit this->message_to_gui(TEXT_MESSAGE_TYPE__WARNING, err);
  qDebug() << err;
}






void RoboTcpServer::data_from_http(const Message &mes)
{
  serial.transaction( name_serial_port, serial_timeout, pars.tx_to_serial(mes) );
}


void RoboTcpServer::incomingConnection(qintptr socketDescriptor)
{
  tcp_s_thread.incoming(socketDescriptor);
}



