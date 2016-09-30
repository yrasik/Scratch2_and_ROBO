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

#include <SPI.h>
#include "printf.h"

#include <Servo.h>
#include <Stepper.h>


#define VERSION 0x01


// change this to the number of steps on your motor
#define STEPS 100

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 7, 6, 5, 4);


#define SCK  13
#define MOSI  11
#define MISO  12
#define nSS  3

Servo servo_0;  // create servo object to control a servo
Servo servo_1;  // create servo object to control a servo
Servo servo_2;  // create servo object to control a servo


int8_t  val_stepper_0;
uint8_t val_servo_0;
uint8_t val_servo_1;
uint8_t val_servo_2;


uint8_t SPI_in_buf [20];
uint8_t SPI_in_buf_pos;


uint8_t SPI_in_exec [20];
uint8_t SPI_in_exec_size;


enum SPI_slave_states
{
  SPI_slave_states__WAIT_MASTER_NSS,
  SPI_slave_states__WAIT_COMMAND_BYTE,
  SPI_slave_states__WAIT_NEXT_BYTE,
  SPI_slave_states__EXEC_COMMAND
} SPI_slave_state;


enum SPI_commands
{
  SPI_command__EMPTY       = 0x00,
  SPI_command__WRITE_ALL   = 0x01,
  SPI_command__GET_STATUS  = 0x02,
  SPI_command__GET_VERSION = 0xA1,
  SPI_command__EMPTY1      = 0xFF
} SPI_command;


enum SYS_statuses
{
  SYS_status__READY    = 0x00,
  SYS_status__BUSY     = 0x01
} SYS_status;





void nSS_state_CHANGE()
{
  uint8_t temp = digitalRead(nSS) & 0x01;

  if(temp == 0)
  {
    SPI.detachInterrupt();
    //Рестартую SPI - модуль. Иначе в колообразном состоянии пребывает,
    //если ранее поступившее кол-во импульсов было не кратно 8.
    SPCR &= ~_BV(SPE); 
    SPCR |= _BV(SPE);
    SPI.attachInterrupt();
  }

  switch(SPI_slave_state)
  {
    case SPI_slave_states__WAIT_MASTER_NSS :
      if(temp == 0)
      {
        SPI_slave_state = SPI_slave_states__WAIT_COMMAND_BYTE;
        SPI_in_buf_pos = 0;
      }
      else
      {
        //неведомая фигня
      }
      break;    
    case SPI_slave_states__WAIT_NEXT_BYTE :
      if(temp == 0)
      {
        //неведомая фигня
        SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;
      }
      else
      { 
        SPI_in_exec_size = SPI_in_buf_pos + 1;
        memcpy(SPI_in_exec, SPI_in_buf, SPI_in_exec_size ); //FIXME  (SPI_buf_pos + 1)
        SPI_slave_state = SPI_slave_states__EXEC_COMMAND;
      }
      break;
    //default:
      //неведомая фигня
  }
}


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  uint8_t temp = SPDR;  // grab byte from SPI Data Register

  switch(SPI_slave_state)
  {
    case SPI_slave_states__WAIT_MASTER_NSS :
      break;
    case SPI_slave_states__WAIT_COMMAND_BYTE :
      SPI_command = (SPI_commands)temp;
      switch(SPI_command)
      {
        case SPI_command__WRITE_ALL :
          SPI_slave_state = SPI_slave_states__WAIT_NEXT_BYTE;
          break; 
        case SPI_command__GET_STATUS :
          SPDR = SYS_status; 
          break;
        case SPI_command__GET_VERSION :
          SPDR = VERSION;
          break;
        default :
          SPDR = 0xFF;
        //неведомая фигня
      }   
      break;
    case SPI_slave_states__WAIT_NEXT_BYTE :
      switch(SPI_command)
      {
        case SPI_command__WRITE_ALL :
          SPI_in_buf [SPI_in_buf_pos++] = temp;
          SPI_in_buf_pos = ( SPI_in_buf_pos < sizeof(SPI_in_buf) ) ? SPI_in_buf_pos : (sizeof(SPI_in_buf) - 1);
          break;
        case SPI_command__GET_STATUS :
          break;
        case SPI_command__GET_VERSION :      
          break;          
        //default :
        //неведомая фигня
      }
      break;
    case SPI_slave_states__EXEC_COMMAND :
      //SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;
      break; 
    //default:
      //неведомая фигня
  }

}  // end of interrupt routine SPI_STC_vect



void setup() {

  Serial.begin (115200);   // debugging
  printf_begin();
  
  servo_0.attach(8);  // attaches the servo on pin 9 to the servo object
  servo_1.attach(9);  // attaches the servo on pin 9 to the servo object
  servo_2.attach(2);  // attaches the servo on pin 9 to the servo object

  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(100);

  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT); // have to send on master in, *slave out*
  pinMode(nSS, INPUT_PULLUP);
  SPCR &= ~_BV(MSTR);// turn on SPI in slave mode
  SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;
  SPI_command = SPI_command__EMPTY;
  SYS_status = SYS_status__BUSY;
  attachInterrupt(digitalPinToInterrupt(nSS), nSS_state_CHANGE, CHANGE);
}


void loop() 
{
  if(SPI_slave_state == SPI_slave_states__EXEC_COMMAND)
  {
    switch(SPI_command)
    {
      case SPI_command__WRITE_ALL :
        val_stepper_0 = (int8_t)SPI_in_exec[0];
        val_servo_0 = SPI_in_exec[1];     // scale it to use it with the servo (value between 0 and 180)
        val_servo_1 = SPI_in_exec[2];     // scale it to use it with the servo (value between 0 and 180)
        val_servo_2 = SPI_in_exec[3];     // scale it to use it with the servo (value between 0 and 180)

        SPI_command = SPI_command__EMPTY;
        SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;     
        stepper.step(val_stepper_0);      
        servo_0.write(val_servo_0);
        servo_1.write(val_servo_1);
        servo_2.write(val_servo_2);
printf("WRITE_ALL\n\r");   
        delay(100);                           // waits for the servo to get there
        break;
        case SPI_command__GET_STATUS :

        SPI_command = SPI_command__EMPTY;
        SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;  
printf("GET_STATUS\n\r");        
          break; 
        case SPI_command__GET_VERSION :

        SPI_command = SPI_command__EMPTY;
        SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;  
printf("GET_VERSION\n\r");        
             
        default :
          //неведомая фигня
          SPI_command = SPI_command__EMPTY;
          SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS; 
printf("default :\n\r");          
      }
    }
  }


