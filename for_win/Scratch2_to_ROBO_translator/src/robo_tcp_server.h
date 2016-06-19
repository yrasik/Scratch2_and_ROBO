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



#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QQueue>

#include "serial_thread.h"
#include "parser.h"
#include "tcp_sockets.h"









class RoboTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit RoboTcpServer(QObject *parent = 0);
    ~RoboTcpServer();
    signed char startServer(const QString &name_serial_port);
    void stopServer(void);
signals:
    void slot_showResponse(const QByteArray &s);
    void slot_processError(void);
    void slot_processTimeout(void);
    void message_to_gui(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes);
public slots:
    void data_from_http(const Message &mes);

private slots:
    void showResponse(const QByteArray &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);
    void message_from_parser(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes);
    void message_from_serial(unsigned int mes_type, const QString &mes);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    TcpSockets tcp_s_thread;
    parser pars;
    SerialThread serial;
    QString name_serial_port;
};

#endif // MYSERVER_H
