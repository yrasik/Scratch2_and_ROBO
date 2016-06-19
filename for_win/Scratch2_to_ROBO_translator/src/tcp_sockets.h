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

#ifndef TCPSOCKETTHREAD_H
#define TCPSOCKETTHREAD_H

#include "parser.h"
#include <QTcpSocket>
#include <QDebug>


class TcpSockets : public QObject
{
    Q_OBJECT
public:
    explicit TcpSockets(QObject *parent = 0);
    ~TcpSockets();

    void run();

    void incoming(qintptr ID);
    void data_from_main_app(const Message &s);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void message_to_gui(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes);
    void data_to_main_app(const Message &s);

private slots:
    void slotReadClient();

private:
    QMap<qintptr, QTcpSocket *> SClients;
    qintptr ID;

    bool incoming_state;
    bool data_from_main_app_state;
    class Message mess_from_app;

};

#endif // TCPSOCKETTHREAD_H
