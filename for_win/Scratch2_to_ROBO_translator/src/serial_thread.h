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


#ifndef SERIAL_THREAD_H
#define SERIAL_THREAD_H


#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <QDebug>
#include <QtSerialPort/QtSerialPort>
#include <QTime>

#include "support.h"

class SerialThread : public QThread
{
    Q_OBJECT

public:
    SerialThread(QObject *parent = 0);
    ~SerialThread();

    void transaction(const QString &portName, int waitTimeout, const QByteArray &request);

    void run();

signals:
    void response(const QByteArray &s);
    void error(const QString &s);
    void timeout(const QString &s);
    void message_to_gui(unsigned int mes_type, const QString &mes);

private:
    QString portName;
    int waitTimeout;
    QMutex mutex;
    QWaitCondition cond;
    bool quit;
    QByteArray request;
    QSerialPort serial;

};




#endif // SERIAL_THREAD_H
