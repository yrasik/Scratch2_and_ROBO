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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QObject>
#include <QByteArray>
#include <QDebug>


#include "parser.h"
#include "serial_thread.h"
#include "robo_tcp_server.h"


class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;

class QTextBrowser;
class QPushButton;
class QPushButton;
class QTcpServer;


class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

private slots:
    void showResponse(const QByteArray &s);
    void processError(void);
    void processTimeout(void);

    void message_from_server(enum TEXT_MESSAGE_TYPE mes_type, const QString &mes);

    void on_starting_clicked();
    void on_stoping_clicked();
private:
    QLabel *serialPortLabel;
    QComboBox *serialPortComboBox;
    QTextBrowser *textinfo;
    QPushButton *stoping;
    QPushButton *starting;

    bool server_status;

    RoboTcpServer server;
    bool transaction_to_serial;


};

#endif // DIALOG_H
