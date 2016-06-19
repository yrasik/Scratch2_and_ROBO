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



#include "serial_thread.h"



QT_USE_NAMESPACE


SerialThread::SerialThread(QObject *parent)
    : QThread(parent), waitTimeout(0), quit(false)
{

}


SerialThread::~SerialThread()
{

}


void SerialThread::run()
{

}



void SerialThread::transaction(const QString &portName, int waitTimeout, const QByteArray &request)
{
  this->waitTimeout = waitTimeout;
  this->request = request;

  if(this->portName != portName)
  {
    this->portName = portName;
    serial.close();
    serial.setPortName(this->portName);

    if (!serial.open(QIODevice::ReadWrite))
    {
      QString  err = "ERROR 88: Не могу открыть " + this->portName + "\n";
      emit message_to_gui((unsigned int)TEXT_MESSAGE_TYPE__ERROR, err);
        qDebug() << err;

      emit error(tr("Can't open %1, error code %2")
                       .arg(this->portName).arg(serial.error()));
      return;
    }
    else
    {
      QString  err = "Последовательный порт " + this->portName + " открыт\n";
       emit message_to_gui((unsigned int)TEXT_MESSAGE_TYPE__GOOD, err);
       qDebug() << err;
    }

    serial.setBaudRate(115200,QSerialPort::AllDirections);
  }


  serial.write(this->request);
  if (serial.waitForBytesWritten(this->waitTimeout))
  {
    // read response
    if (serial.waitForReadyRead(this->waitTimeout))
    {
      QByteArray responseData = serial.readAll();
      while (serial.waitForReadyRead(10)) //Без этого с Arduino UNO связи нет
                responseData += serial.readAll();
//                qDebug() << "<--" << responseData.toHex();
      emit this->response(responseData);
    }
    else
    {
      QString  err = "ERROR 89: Истёк интервал ожидания ответа от робота\n";
      emit message_to_gui((unsigned int)TEXT_MESSAGE_TYPE__ERROR, err);
        qDebug() << err;

      emit timeout(tr("Wait read response timeout %1")
                         .arg(QTime::currentTime().toString()));
    }
  }

}

