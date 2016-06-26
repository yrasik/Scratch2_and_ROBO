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


#include "dialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QtWidgets/QTextBrowser>


QT_USE_NAMESPACE

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , serialPortLabel(new QLabel(tr("Последовательный порт:")))
    , serialPortComboBox(new QComboBox())
    , textinfo(new QTextBrowser(this))
    , stoping(new QPushButton(tr("Остановка")))
    , starting(new QPushButton(tr("Запуск")))
    , Label_serial_timeout(new QLabel(tr("Ожидание, мС:")))
    , LineEdit_serial_timeout(new QLineEdit(tr("300")))

{

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        serialPortComboBox->addItem(info.portName());



    QHBoxLayout *HBoxLayout_serial = new QHBoxLayout;
    HBoxLayout_serial->addWidget(serialPortLabel);
    HBoxLayout_serial->addWidget(serialPortComboBox);
    QSpacerItem *horizontalSpacer_serial = new QSpacerItem(40, 18, QSizePolicy::Expanding, QSizePolicy::Minimum);
    HBoxLayout_serial->addItem(horizontalSpacer_serial);
    HBoxLayout_serial->addWidget(Label_serial_timeout);
    LineEdit_serial_timeout->setMaximumWidth(40);
    HBoxLayout_serial->addWidget(LineEdit_serial_timeout);

    QHBoxLayout *HBoxLayout_keys = new QHBoxLayout;
    HBoxLayout_keys->addWidget(stoping);
    QSpacerItem *horizontalSpacer_keys = new QSpacerItem(40, 18, QSizePolicy::Expanding, QSizePolicy::Minimum);
    HBoxLayout_keys->addItem(horizontalSpacer_keys);
    HBoxLayout_keys->addWidget(starting);

    QVBoxLayout *VBoxLayout_main = new QVBoxLayout;
    VBoxLayout_main->addLayout(HBoxLayout_serial);
    VBoxLayout_main->addWidget(textinfo);
    VBoxLayout_main->addLayout(HBoxLayout_keys);

    setLayout(VBoxLayout_main);



    setWindowTitle(tr("Scratch2 to ROBO translator"));
    serialPortComboBox->setFocus();

    connect(&server, &RoboTcpServer::slot_showResponse, this, &Dialog::showResponse);
    connect(&server, &RoboTcpServer::slot_processError, this, &Dialog::processError);
    connect(&server, &RoboTcpServer::slot_processTimeout, this, &Dialog::processTimeout);
    connect(&server, &RoboTcpServer::message_to_gui, this, &Dialog::message_from_server);

    connect(stoping, &QPushButton::clicked, this, &Dialog::on_stoping_clicked);
    connect(starting, &QPushButton::clicked, this, &Dialog::on_starting_clicked);

    stoping->setEnabled(false);

    textinfo->setTextColor(Qt::black);

    server_status = false;
    transaction_to_serial = false;
}


void Dialog::message_from_server(TEXT_MESSAGE_TYPE mes_type, const QString &mes)
{
  switch( mes_type )
  {
    case TEXT_MESSAGE_TYPE__GOOD :
      break;
    case TEXT_MESSAGE_TYPE__ERROR :
      textinfo->setTextColor(Qt::darkRed);
      break;
    case TEXT_MESSAGE_TYPE__WARNING :
      textinfo->setTextColor(Qt::darkBlue);
      break;
    case TEXT_MESSAGE_TYPE__GREEN :
      textinfo->setTextColor(Qt::darkGreen);
      break;

  }

  textinfo->insertPlainText(mes);
  textinfo->setTextColor(Qt::black);
}





void Dialog::showResponse(const QByteArray &s)
{
  //textinfo->append(("<-" + s.toHex()) );

  if( transaction_to_serial == false)
  {
    message_from_server(TEXT_MESSAGE_TYPE__GOOD, "Установлен обмен с роботом\n");
    transaction_to_serial = true;
  }
}


void Dialog::processError(void)
{
  transaction_to_serial = false;
}


void Dialog::processTimeout(void)
{
  transaction_to_serial = false;
}


void Dialog::on_starting_clicked()
{
  QString str_timeout = LineEdit_serial_timeout->text();
  int timeout = str_timeout.toInt();

  if( (timeout <= 0) || (timeout > 10000) )
  {
    timeout = 300;
  }

  LineEdit_serial_timeout->setText( QString::number(timeout) );

  if( server.startServer( serialPortComboBox->currentText(), timeout ) == 0 );
  {
    server_status = true;
    starting->setEnabled(false);
    stoping->setEnabled(true);
    serialPortComboBox->setEnabled(false);
    transaction_to_serial = false;
    LineEdit_serial_timeout->setEnabled(false);
  }
}


void Dialog::on_stoping_clicked()
{
    if(server_status == true )
    {
      server.stopServer();
      server_status = false;
      starting->setEnabled(true);
      stoping->setEnabled(false);
      serialPortComboBox->setEnabled(true);
      transaction_to_serial = false;
      LineEdit_serial_timeout->setEnabled(true);
    }
}

