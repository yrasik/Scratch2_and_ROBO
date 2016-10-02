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


#include "parser.h"


#define RUS_UTF8__OTKL   "%D0%9E%D1%82%D0%BA%D0%BB."  /*Откл. - в такой вид преобразует клиент скрэтча*/
#define RUS_UTF8__VKL    "%D0%92%D0%BA%D0%BB."   /*Вкл. - в такой вид преобразует клиент скрэтча*/



parser::parser(QObject *parent) :
QObject(parent)
{
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__poll, SCRATCH_BLOCK_TYPE__EMPTY, "/poll") ); //Пусть на первом месте всегда будет
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__reset_all, SCRATCH_BLOCK_TYPE__EMPTY, "/reset_all") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__move_front, SCRATCH_BLOCK_TYPE__W, "/move_front", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__move_left, SCRATCH_BLOCK_TYPE__W, "/move_left", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__move_right, SCRATCH_BLOCK_TYPE__W, "/move_right", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__move_back, SCRATCH_BLOCK_TYPE__W, "/move_back", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__servo_0_degrees, SCRATCH_BLOCK_TYPE__W, "/servo_0_degrees", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__servo_1_degrees, SCRATCH_BLOCK_TYPE__W, "/servo_1_degrees", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__servo_2_degrees, SCRATCH_BLOCK_TYPE__W, "/servo_2_degrees", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__servo_3_degrees, SCRATCH_BLOCK_TYPE__W, "/servo_3_degrees", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__led_0, SCRATCH_BLOCK_TYPE__EMPTY, "/led_0", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__led_1, SCRATCH_BLOCK_TYPE__EMPTY, "/led_1", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__led_2, SCRATCH_BLOCK_TYPE__EMPTY, "/led_2", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__piezo_tone, SCRATCH_BLOCK_TYPE__EMPTY, "/piezo_tone", 2) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__line_left, SCRATCH_BLOCK_TYPE__R, "/line_left") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__line_right, SCRATCH_BLOCK_TYPE__R, "/line_right") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__wall_left, SCRATCH_BLOCK_TYPE__R, "/wall_left") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__wall_right, SCRATCH_BLOCK_TYPE__R, "/wall_right") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__u_sonic_left, SCRATCH_BLOCK_TYPE__R, "/u_sonic_left") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__u_sonic_center, SCRATCH_BLOCK_TYPE__R, "/u_sonic_center") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__u_sonic_right, SCRATCH_BLOCK_TYPE__R, "/u_sonic_right") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__gyro_X, SCRATCH_BLOCK_TYPE__R, "/gyro_X") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__gyro_Y, SCRATCH_BLOCK_TYPE__R, "/gyro_Y") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__gyro_Z, SCRATCH_BLOCK_TYPE__R, "/gyro_Z") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__photo, SCRATCH_BLOCK_TYPE__R, "/photo") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__thermo, SCRATCH_BLOCK_TYPE__R, "/thermo") );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__move_speed, SCRATCH_BLOCK_TYPE__EMPTY, "/move_speed", 1) );
  scratch_tags.push_back( scratch_tag::init(SCRATCH_TAG__stepper_0_steps, SCRATCH_BLOCK_TYPE__W, "/stepper_0_steps", 1) );


  Robo_ID = "ROBO:";

  canvas = "HTTP/1.1 200 OK\r\n";
  canvas += "Content-Type: text/html; charset=ISO-8859-1\r\n";
  canvas += "Access-Control-Allow-Origin: *\r\n\r\n";

  packet_num_tx = 0;
}


parser::~parser()
{

}


signed char parser::read_from_http(const QString &str)
{
  to_serial.clear();
  int start, end;

  if( str.indexOf("GET") == -1 ) //В описании клиента Scratch2 сказано что он использует запросы типа GET
  {
    to_serial += (char)SCRATCH_TAG__poll;
    QString  err = "ERROR 123: Странный пакет от Scratch :" + str + "\n";
    emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
    qDebug() << err;
    return -123;
  }

  QVector<scratch_tag>::iterator iter;

  for(iter = scratch_tags.begin(); iter < scratch_tags.end(); iter++ )
  {
    if( (start = str.indexOf((*iter).str)) != -1 )  //нашли знакомый тег....
    {

      switch( (unsigned int)(*iter).tag ) // SCRATCH_TAG
      {
        case SCRATCH_TAG__poll : //52 4F 42 4F 3A 00 01 00
          to_serial += (char)SCRATCH_TAG__poll;
          break;
        case SCRATCH_TAG__reset_all :
          to_serial += (char)SCRATCH_TAG__reset_all;
          break;
        case SCRATCH_TAG__move_front : 
        case SCRATCH_TAG__move_left :
        case SCRATCH_TAG__move_right :
        case SCRATCH_TAG__move_back :
//{
//QString  err = "-->" + str + "\n";
//emit this->message_to_gui(TEXT_MESSAGE_TYPE__WARNING, err);
//qDebug() << err;
//      }

          start += (*iter).str.size() + 1;
          if( (end = str.indexOf("/", start)) != -1 )
          {
            bool ok;
            QString str_arg = str.mid( start, (end - start) );
            int arg_0_wait_id = str_arg.toInt(&ok); //Идентификатор номера задержки
            if( ok == true )
            {
              start = end + 1;
              if( (end = str.indexOf(" HTTP")) != -1 )
              {
                str_arg.clear();
                str_arg = str.mid( start, (end - start) );
                float fl_arg_1_wait_value_sec = str_arg.toFloat(&ok);
                unsigned long arg_1_wait_value_msec = fl_arg_1_wait_value_sec * 1000; //задержку из секунд перевели в милисекунды и преобразовали в 4-х байтное целое
                if( ok == true )
                {
                  to_serial += (char)((*iter).tag);
                  to_serial += (char)((arg_0_wait_id >> 8) & 0xFF); //HI
                  to_serial += (char)( arg_0_wait_id       & 0xFF); //LO
                  to_serial += (char)((arg_1_wait_value_msec  >> 24) & 0xFF); //HIHI  //Старшим байтом вперёд
                  to_serial += (char)((arg_1_wait_value_msec  >> 16) & 0xFF); //HILO
                  to_serial += (char)((arg_1_wait_value_msec  >> 8 ) & 0xFF); //LOHI
                  to_serial += (char)( arg_1_wait_value_msec         & 0xFF); //LOLO
                }
                else
                {
                  QString  err = "ERROR 30: Не смог прочитать аргумент №2:" + str + "\n";
                  emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                  qDebug() << err;
                  return -30;
                }
              }
              else
              {
                QString  err = "ERROR 31: Странный пакет от Scratch (не хватает тега 'HTTP') :" + str + "\n";
                emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                qDebug() << err;
                return -31;
              }
            }
            else
            {
              QString  err = "ERROR 32: Не смог прочитать аргумент №1: " + str + "\n";
              emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
              qDebug() << err;
              return -32;
            }
          }
          else
          {
            QString  err = "ERROR 33: Странный пакет от Scratch :" + str + "\n";
            emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
            qDebug() << err;
            return -33;
          }
          break;       
        case SCRATCH_TAG__servo_0_degrees : //В случае сервомотора - вторым аргументом является угол поворота, а не задержка. Как микроконтроллер отработает, так уберёт _busy из ответа
        case SCRATCH_TAG__servo_1_degrees :
        case SCRATCH_TAG__servo_2_degrees :
        case SCRATCH_TAG__servo_3_degrees :
        case SCRATCH_TAG__stepper_0_steps :
          start += (*iter).str.size() + 1;
          if( (end = str.indexOf("/", start)) != -1 )
          {
            bool ok;
            QString str_arg = str.mid( start, (end - start) );
            int arg_0_wait_id = str_arg.toInt(&ok); //Идентификатор номера задержки
            if( ok == true )
            {
              start = end + 1;
              if( (end = str.indexOf(" HTTP")) != -1 )
              {
                str_arg.clear();
                str_arg = str.mid( start, (end - start) );
                uint8_t arg_1_wait_value_degres = (uint8_t)(str_arg.toInt(&ok));
                if( ok == true )
                {
                  to_serial += (char)((*iter).tag);
                  to_serial += (char)((arg_0_wait_id >> 8) & 0xFF); //HI
                  to_serial += (char)( arg_0_wait_id       & 0xFF); //LO
                  to_serial += (char)(arg_1_wait_value_degres);
                }
                else
                {
                  QString  err = "ERROR 40: Не смог прочитать аргумент №2:" + str + "\n";
                  emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                  qDebug() << err;
                  return -40;
                }
              }
              else
              {
                QString  err = "ERROR 41: Странный пакет от Scratch (не хватает тега 'HTTP') :" + str + "\n";
                emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                qDebug() << err;
                return -41;
              }
            }
            else
            {
              QString  err = "ERROR 42: Не смог прочитать аргумент №1: " + str + "\n";
              emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
              qDebug() << err;
              return -42;
            }
          }
          else
          {
            QString  err = "ERROR 43: Странный пакет от Scratch :" + str + "\n";
            emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
            qDebug() << err;
            return -43;
          }
          break;
        case SCRATCH_TAG__led_0 : //52 4F 42 4F 3A 00 03 0A 00 01 - как должно быть в итоге
        case SCRATCH_TAG__led_1 : //52 4F 42 4F 3A 00 03 0B 00 01
        case SCRATCH_TAG__led_2 :
          start += (*iter).str.size() + 1;
          to_serial += (char)((*iter).tag);
          to_serial += (char)0; //HI
          if( (end = str.indexOf(RUS_UTF8__OTKL)) != -1 ) //Откл.
          {
            to_serial += (char)0; //LO
          }
          else
          {
            if( (end = str.indexOf(RUS_UTF8__VKL)) != -1 ) //Вкл.
            {
              to_serial += (char)1; //LO
            }
            else
            {
              QString  err = "ERROR 2: Странный пакет от Scratch :" + str + "\n";
              emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
              qDebug() << err;
              return -2;
            }
          }
          break;
        case SCRATCH_TAG__move_speed :
          start += (*iter).str.size() + 1;
          to_serial += (char)((*iter).tag);

          if( (end = str.indexOf(" HTTP", start)) != -1 )
          {
            bool ok;
            QString str_arg = str.mid( start, (end - start) );
            unsigned char speed = str_arg.toInt(&ok); //Идентификатор номера задержки
            if( ok == true )
            {
              to_serial += speed;
            }
            else
            {
              QString  err = "ERROR 15: Странный пакет от Scratch :" + str + "\n";
              emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
              qDebug() << err;
              return -15;
            }
          }
          else
          {
            QString  err = "ERROR 14: Странный пакет от Scratch :" + str + "\n";
            emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
            qDebug() << err;
            return -14;
          }
          break;
        case SCRATCH_TAG__piezo_tone :
          start += (*iter).str.size() + 1;
          if( (end = str.indexOf("/", start)) != -1 )
          {
            bool ok;
            QString str_arg = str.mid( start, (end - start) );
            float fl_arg_0_tone_value_hz = str_arg.toFloat(&ok);
            unsigned long arg_0_tone_value_hz = fl_arg_0_tone_value_hz; //частоту в Гц преобразовали в 4-х байтное целое
            if( ok == true )
            {
              start = end + 1;
              if( (end = str.indexOf(" HTTP")) != -1 )
              {
                str_arg.clear();
                str_arg = str.mid( start, (end - start) );
                float fl_arg_1_wait_value_sec = str_arg.toFloat(&ok);
                unsigned long arg_1_wait_value_msec = fl_arg_1_wait_value_sec * 1000; //задержку из секунд перевели в милисекунды и преобразовали в 4-х байтное целое
                if( ok == true )
                {
                  to_serial += (char)((*iter).tag);
                  to_serial += (char)((arg_0_tone_value_hz  >> 24) & 0xFF); //HIHI  //Старшим байтом вперёд
                  to_serial += (char)((arg_0_tone_value_hz  >> 16) & 0xFF); //HILO
                  to_serial += (char)((arg_0_tone_value_hz  >> 8 ) & 0xFF); //LOHI
                  to_serial += (char)( arg_0_tone_value_hz         & 0xFF); //LOLO
                  to_serial += (char)((arg_1_wait_value_msec  >> 24) & 0xFF); //HIHI  //Старшим байтом вперёд
                  to_serial += (char)((arg_1_wait_value_msec  >> 16) & 0xFF); //HILO
                  to_serial += (char)((arg_1_wait_value_msec  >> 8 ) & 0xFF); //LOHI
                  to_serial += (char)( arg_1_wait_value_msec         & 0xFF); //LOLO
                }
                else
                {
                  QString  err = "ERROR 11: Не смог прочитать аргумент №2:" + str + "\n";
                  emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                  qDebug() << err;
                  return -11;
                }
              }
              else
              {
                QString  err = "ERROR 12: Странный пакет от Scratch (не хватает тега 'HTTP') :" + str + "\n";
                emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
                qDebug() << err;
                return -12;
              }
            }
            else
            {
              QString  err = "ERROR 13: Не смог прочитать аргумент №1:" + str + "\n";
              emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
              qDebug() << err;
              return -13;
            }
          }
          else
          {
            QString  err = "ERROR 14: Странный пакет от Scratch : " + str + "\n";
            emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
            qDebug() << err;
            return -14;
          }
          break;
      }

      break;
    }
  }
// qDebug() << "----->"  << str;


//{
//  QString  err = "ok\n";
//  emit this->message_to_gui(TEXT_MESSAGE_TYPE__WARNING, err);
//  qDebug() << err;
//}
  return 0;
}


signed char parser::write_to_http(QByteArray &arr)
{
  arr.clear();
  arr += canvas;
  arr += to_scratch;
  to_scratch.clear();
// qDebug() << "<-- " << arr;

  return 0;
}


signed char parser::write_to_serial(QByteArray &arr)
{
  arr.clear();
  arr = Robo_ID;
  arr += packet_num_tx;
  arr += (unsigned char)(to_serial.size()); //Размер
  arr += to_serial;

 // qDebug() << "--> " << arr;
  return 0;
}


signed char parser::read_from_serial(const QByteArray &arr, unsigned char packet_num)
{
  int i;

 // qDebug() << "<---- " << arr;

  if ( (i = arr.indexOf( Robo_ID ) ) != -1 ) //Вдруг начало пакета не совпадает с началом сообщения...
  {
    i += Robo_ID.size();

    if( arr.size() < i + (Robo_ID.size() + 1 + 1) ) //Пакет от платы минимального размера
    {
      QString  err = "ERROR 5: Странный пакет от робота :" + arr.toHex() + "\n";
      emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
      qDebug() << err;
      return -5;
    }

    if( (unsigned char)(arr.at(i)) != packet_num ) //т.к. имеем дело с потоком последовательного приёмопередатчика, то нужно осмотрительно относиться к этому условию...
    {
      QString  err =  "ERROR 100: Переданный пакет потерян: \n";
               err += "  ожидаемый номер: 0x" + QString::number(packet_num, 16) + "\n";
               err += "  принятый номер: 0x" + QString::number((unsigned char)arr.at(i), 16) + "\n";
      emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
      qDebug() << err;
     // return -100;
    }

    if( arr.at(++i) > 0 ) // Размер содержательной части пакета
    {
      if( arr.size() < (i + arr.at(i)) ) //Проверка полноценности содержательной части
      {
        QString  err = "ERROR 23: Странный пакет от робота :" + arr.toHex() + "\n";
        emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
        qDebug() << err;
        return -23;
      }

       to_scratch.clear();
       switch ( arr.at(++i) )
       {
         case SCRATCH_TAG__move_front :
         case SCRATCH_TAG__move_left :
         case SCRATCH_TAG__move_right :
         case SCRATCH_TAG__move_back :
         case SCRATCH_TAG__servo_0_degrees :
         case SCRATCH_TAG__servo_1_degrees :
         case SCRATCH_TAG__servo_2_degrees :
         case SCRATCH_TAG__servo_3_degrees :
         {
           unsigned int num  =  ( (unsigned int)arr.at(++i) << 8) & 0xFF00;
                        num |=  ( (unsigned int)arr.at(++i)     ) & 0x00FF;
           to_scratch += "_busy " + QByteArray::number( num ) + "\r\n";
         }
         case SCRATCH_TAG__reset_all :
         case SCRATCH_TAG__poll :
         case SCRATCH_TAG__move_speed :
           to_scratch += "line_left " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "line_right " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "wall_left " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "wall_right " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "u_sonic_left " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "u_sonic_center " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "u_sonic_right " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "gyro_X " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "gyro_Y " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "gyro_Z " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "photo " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           to_scratch += "thermo " + QByteArray::number( (int)(arr.at(++i)) ) + "\r\n";
           break;
       }
     }
  }
  else
  {
    QString  err = "ERROR 6: Странный пакет от робота (нет начала пакета) :" + arr.toHex() + "\n";
    emit this->message_to_gui(TEXT_MESSAGE_TYPE__ERROR, err);
    qDebug() << err;
    return -6;
  }

  return 0;
}

























QByteArray parser::tx_to_serial(const Message &arr)
{
  QByteArray temp;

  read_from_http( arr.Data );
  write_to_serial(temp);
  packet_num_rx = packet_num_tx;
  packet_num_tx++; // //Не совсем понятно почему в этом месте, а не ниже.... Для Arduino UNO с оригинальной микрухой ком порта прокатывает

  return temp;
}


Message parser::rx_serial_to_web(const QByteArray &arr)
{
  Message mes_to_web;

  write_to_http(mes_to_web.Data);

  mes_to_web.time = QTime::currentTime();
  mes_to_web.ID = 0;

  return mes_to_web;
}


signed char parser::rx_serial_to_web_check(const QByteArray &arr)
{

  return read_from_serial(arr, packet_num_rx);
}
