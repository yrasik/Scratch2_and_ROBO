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


#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QObject>
#include <QTextStream>
#include <QTextCodec>
#include <QByteArray>
#include <QVector>
#include <QDebug>
#include <QQueue>
#include <QTime>

#include "support.h"


class parser : public QObject
{
    Q_OBJECT
public:
    parser(QObject *parent = 0);
    ~parser();

    QByteArray tx_to_serial(const Message &arr);

    Message rx_serial_to_web(const QByteArray &arr);

    signed char rx_serial_to_web_check(const QByteArray &arr);



signals:
    void message_to_gui(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes);

public slots:


private:
    signed char read_from_http(const QString &str);
    signed char write_to_serial(QByteArray &arr);

    signed char read_from_serial(const QByteArray &arr, unsigned char packet_num);
    signed char write_to_http(QByteArray &arr);

    QVector<scratch_tag> scratch_tags;
    QByteArray canvas;
    QByteArray Robo_ID;
    enum SCRATCH_TAG tag_rx;

    QByteArray to_serial;
    QByteArray from_serial;

    QByteArray to_scratch;

    unsigned char packet_num_tx;
    unsigned char packet_num_rx;


    QQueue <Message> quere_from_web;

    QQueue <Message> quere_to_serial;


};

#endif // PARSER_H
