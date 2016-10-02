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


#ifndef SUPPORT_H
#define SUPPORT_H

#include <QString>
#include <QTime>
#include <QByteArray>


class Message
{
public:
  Message()
  {
    ID = 0;
    time = QTime::currentTime();
  }

  static Message init(qintptr ID_, QByteArray Data_, QTime time_)
  {
    class Message temp;

    temp.ID = ID_;
    temp.Data = Data_;
    temp.time = time_;
    return temp;
  }
  Message operator=(const Message &mes2)
  {
    ID = mes2.ID;
    Data = mes2.Data;
    time = mes2.time;
    return *this;
  }

public:
  qintptr ID;
  QByteArray Data;
  QTime time;
};










enum SCRATCH_TAG
{
  SCRATCH_TAG__poll = 0,
  SCRATCH_TAG__reset_all,
  SCRATCH_TAG__move_front,
  SCRATCH_TAG__move_left,
  SCRATCH_TAG__move_right,
  SCRATCH_TAG__move_back,
  SCRATCH_TAG__servo_0_degrees,
  SCRATCH_TAG__servo_1_degrees,
  SCRATCH_TAG__servo_2_degrees,
  SCRATCH_TAG__servo_3_degrees,
  SCRATCH_TAG__led_0,
  SCRATCH_TAG__led_1,
  SCRATCH_TAG__led_2,
  SCRATCH_TAG__piezo_tone,
  SCRATCH_TAG__line_left,
  SCRATCH_TAG__line_right,
  SCRATCH_TAG__wall_left,
  SCRATCH_TAG__wall_right,
  SCRATCH_TAG__u_sonic_left,
  SCRATCH_TAG__u_sonic_center,
  SCRATCH_TAG__u_sonic_right,
  SCRATCH_TAG__gyro_X,
  SCRATCH_TAG__gyro_Y,
  SCRATCH_TAG__gyro_Z,
  SCRATCH_TAG__photo,
  SCRATCH_TAG__thermo,
  SCRATCH_TAG__move_speed,
  SCRATCH_TAG__stepper_0_steps
};


enum SCRATCH_BLOCK_TYPE
{
  SCRATCH_BLOCK_TYPE__EMPTY = 0,  //" " - command block
  SCRATCH_BLOCK_TYPE__W,      //"w" - command block that waits
  SCRATCH_BLOCK_TYPE__R,      //"r" - number reporter block (round ends)
  SCRATCH_BLOCK_TYPE__B       //"b" - boolean reporter block (pointy ends)
};


enum TEXT_MESSAGE_TYPE
{
  TEXT_MESSAGE_TYPE__GOOD = 0,
  TEXT_MESSAGE_TYPE__ERROR,
  TEXT_MESSAGE_TYPE__WARNING,
  TEXT_MESSAGE_TYPE__GREEN
};


class scratch_tag
{
public:
  static scratch_tag init(enum SCRATCH_TAG tag_, enum SCRATCH_BLOCK_TYPE type_, QString str_, unsigned int param_num_ = 0)
  {
    class scratch_tag temp;

    temp.tag = tag_;
    temp.type = type_;
    temp.str = str_;
    temp.param_num = param_num_;
    return temp;
  }


public:
  enum SCRATCH_TAG tag;
  enum SCRATCH_BLOCK_TYPE type;
  QString str;
  unsigned int param_num;
};




struct sensors_TypeDef
{
unsigned char line_left;
unsigned char line_right;

unsigned char wall_left;
unsigned char wall_right;

unsigned char u_sonic_left;
unsigned char u_sonic_center;
unsigned char u_sonic_right;

unsigned char gyro_X;
unsigned char gyro_Y;
unsigned char gyro_Z;

unsigned char photo;
unsigned char thermo;
};


#endif // SUPPORT_H
