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

{

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        serialPortComboBox->addItem(info.portName());

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(serialPortLabel, 0, 0);
    mainLayout->addWidget(serialPortComboBox, 0, 1);
    mainLayout->addWidget(textinfo, 1, 0, 1, 0);
    mainLayout->addWidget(stoping, 2, 0);
    mainLayout->addWidget(starting,2, 1);

    setLayout(mainLayout);



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
  if( server.startServer( serialPortComboBox->currentText() ) == 0 );
  {
    server_status = true;
    starting->setEnabled(false);
    stoping->setEnabled(true);
    serialPortComboBox->setEnabled(false);
    transaction_to_serial = false;
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
    }
}

