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

#define SERVO_MINIMAL_INCREMENT_VALUE   5

#define STEPPER_MOTOR_PIN_1    7
#define STEPPER_MOTOR_PIN_2    6
#define STEPPER_MOTOR_PIN_3    5
#define STEPPER_MOTOR_PIN_4    4

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, STEPPER_MOTOR_PIN_1, STEPPER_MOTOR_PIN_2, STEPPER_MOTOR_PIN_3, STEPPER_MOTOR_PIN_4);

void stepper_pins_to_low(void); //Для шаговика с редуктором, чтобы ток не кушал в режиме простоя



#define SCK  13
#define MOSI  11
#define MISO  12
#define nSS  3

Servo servo_0;  // create servo object to control a servo
Servo servo_1;  // create servo object to control a servo
Servo servo_2;  // create servo object to control a servo
Servo servo_3;  // create servo object to control a servo


int8_t  val_stepper_0;
uint8_t val_servo[4];

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


#define SERVO_0_MIN   0
#define SERVO_0_MAX   140

#define SERVO_1_MIN   0
#define SERVO_1_MAX   180

#define SERVO_2_MIN   0
#define SERVO_2_MAX   150

#define SERVO_3_MIN   10
#define SERVO_3_MAX   120

#define SERVO_DRIVER_GAP 7  


int servo_0_max_min (int val_servo);
int servo_1_max_min (int val_servo);
int servo_2_max_min (int val_servo);
int servo_3_max_min (int val_servo);
void soft_servo_work(Servo* serv, int val_servo); //Для плавности хода сервов


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
  servo_3.attach(19);  // attaches the servo on pin 9 to the servo object

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

//val_servo_0_old = val_servo_0;
        
        val_servo[0] = SPI_in_exec[1];     // scale it to use it with the servo (value between 0 and 180)
        val_servo[1] = SPI_in_exec[2];     // scale it to use it with the servo (value between 0 and 180)
        val_servo[2] = SPI_in_exec[3];     // scale it to use it with the servo (value between 0 and 180)
        val_servo[3] = SPI_in_exec[4];     // scale it to use it with the servo (value between 0 and 180)

        SPI_command = SPI_command__EMPTY;
        SPI_slave_state = SPI_slave_states__WAIT_MASTER_NSS;     
        stepper.step(val_stepper_0);
        stepper_pins_to_low();

        servo_0.write(servo_0_max_min(val_servo[0]));
        soft_servo_work(&servo_1, servo_1_max_min(val_servo[1]));
        soft_servo_work(&servo_2, servo_2_max_min(val_servo[2]));
        soft_servo_work(&servo_3, servo_3_max_min(val_servo[3]));
        
printf("WRITE_ALL 0=%02X  1=%02X 2=%02X 3=%02X\n\r", val_servo[0], val_servo[1], val_servo[2], val_servo[3]);   
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


int servo_0_max_min (int val_servo)
{
   if( val_servo > SERVO_0_MAX )
     return SERVO_0_MAX;

   if( val_servo < SERVO_0_MIN )
     return SERVO_0_MIN;
}


int servo_1_max_min (int val_servo)
{
   if( val_servo > SERVO_1_MAX )
     return SERVO_1_MAX;

   if( val_servo < SERVO_1_MIN )
     return SERVO_1_MIN;
}


int servo_2_max_min (int val_servo)
{
   if( val_servo > SERVO_2_MAX )
     return SERVO_2_MAX;

   if( val_servo < SERVO_2_MIN )
     return SERVO_2_MIN;
}


int servo_3_max_min (int val_servo)
{
   if( val_servo > SERVO_3_MAX )
     return SERVO_3_MAX;

   if( val_servo < SERVO_3_MIN )
     return SERVO_3_MIN;
}


void soft_servo_work(Servo* serv, int val_servo)
{
   int val_servo_old = serv->read(); // returns current pulse width as an angle between 0 and 180
   printf("val_servo_old = %02X\n\r", val_servo_old);
   printf("val_servo = %02X\n\r", val_servo);


   if( (val_servo > val_servo_old) && (val_servo < val_servo_old + SERVO_DRIVER_GAP))
     return; 

   if( ( val_servo < val_servo_old ) && ( val_servo > val_servo_old - SERVO_DRIVER_GAP) )
     return; 

   if( val_servo > val_servo_old )
   {
     while( val_servo > val_servo_old)
     {
       val_servo_old += SERVO_MINIMAL_INCREMENT_VALUE;
       if( val_servo < val_servo_old )
       {
         val_servo = val_servo_old;
       } 
       serv->write(val_servo_old);
       printf("   serv->read() = %02X\n\r", serv->read());
       printf("   val_servo_old = %02X\n\r", val_servo_old);
       delay(300);
     }   
   }
   else
   {
     while( val_servo < val_servo_old)
     {
       val_servo_old -= SERVO_MINIMAL_INCREMENT_VALUE;
       if( val_servo > val_servo_old )
       {
         val_servo = val_servo_old;
       } 
       serv->write(val_servo_old);
       printf("   serv->read() = %02X\n\r", serv->read());
       printf("   val_servo_old = %02X\n\r", val_servo_old);
       delay(300);
     }   
   }
}  


void stepper_pins_to_low(void) //Для шаговика с редуктором, чтобы ток не кушал в режиме простоя
{
  digitalWrite(STEPPER_MOTOR_PIN_1, LOW);
  digitalWrite(STEPPER_MOTOR_PIN_2, LOW);
  digitalWrite(STEPPER_MOTOR_PIN_3, LOW);
  digitalWrite(STEPPER_MOTOR_PIN_4, LOW);
}


