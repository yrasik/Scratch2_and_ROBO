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





#define NRF24_INTERFACE_ENABLE  0


#define HCSR04_CENTER_ENABLE  1
#define HCSR04_LEFT_ENABLE  1
#define HCSR04_RIGHT_ENABLE  0







#if (NRF24_INTERFACE_ENABLE == 1)
  #include "fifo.h"
  #include <SPI.h>
  #include "nRF24L01.h"
  #include "RF24.h"
  #include "printf.h"
#endif


//Для электродвигателей
#define DEFAULT_MOTOR_POWER 200

#define PWM_MOTOR_LEFT_PIN     6
#define PWM_MOTOR_RIGHT_PIN    5
#define WAY_MOTOR_LEFT_PIN     7
#define WAY_MOTOR_RIGHT_PIN    4


//Аналоговые входы (analog pins)
//могут быть использованы как
//цифровые вход/выходы (digital pins).
//Обращение к ним идет по номерам от 14
//(для аналогового входа 0) до 19 (для аналогового входа 5).
  #define DETECTOR_LINE_LEFT_PIN    14  /*A0*/
  #define DETECTOR_LINE_RIGHT_PIN   15  /*A1*/



//Для дальномера
#if ( HCSR04_CENTER_ENABLE == 1 )
  #define HCSR04_CENTER_TRIG_PIN     8
  #define HCSR04_CENTER_ECHO_PIN     3
#endif

#if ( HCSR04_LEFT_ENABLE == 1 )
  #define HCSR04_LEFT_TRIG_PIN     16  /*A2*/
  #define HCSR04_LEFT_ECHO_PIN     2
#endif

#if ( HCSR04_RIGHT_ENABLE == 1 )
  #define HCSR04_RIGHT_TRIG_PIN     18  /*A4*/
  #define HCSR04_RIGHT_ECHO_PIN     17  /*A3*/
#endif




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
  SCRATCH_TAG__move_speed
};


enum RX_STATE
{
  RX_STATE__wait_R = 0,
  RX_STATE__wait_O,
  RX_STATE__wait_B,
  RX_STATE__wait_O1,
  RX_STATE__wait_i,
  RX_STATE__wait_num,
  RX_STATE__wait_size,
  RX_STATE__wait_data,
  RX_STATE__data_accepted
};


uint8_t rx_data_count;
uint8_t RX_state;


uint8_t motorPower;


uint8_t rx_packet_id;
uint8_t rx_packet_num;
uint8_t rx_packet_size;


const uint8_t robo[] = {'R', 'O', 'B', 'O', ':'};
uint8_t command_rx[20];
uint8_t command_tx[29]; ///

uint8_t inByte;

uint32_t started_waiting_at;

uint8_t wait_command;
uint16_t wait_id;

uint32_t delay_ms;
uint32_t time_interval_finish;

uint8_t size_tx;
  


struct sensors_TypeDef
{
  uint8_t line_left;
  uint8_t line_right;

  uint8_t wall_left;
  uint8_t wall_right;

  uint8_t u_sonic_left;
  uint8_t u_sonic_center;
  uint8_t u_sonic_right;

  uint8_t gyro_X;
  uint8_t gyro_Y;
  uint8_t gyro_Z;

  uint8_t photo;
  uint8_t thermo;
} sensors;


//#define led_0  13  //SPI интерфейс nRF






#if (NRF24_INTERFACE_ENABLE == 1)
  typedef struct
  {
    uint8_t size;
    uint8_t Buf[32];
  } RF24_Buf_TypeDef;


  RF24_Buf_TypeDef   Buf_From_RF24;
  RF24_Buf_TypeDef   Buf_To_RF24;

  FIFO(256) fifo_in;

  // Set up nRF24L01 radio on SPI bus plus pins 9 & 10
  RF24 radio(9,10);

  // Radio pipe addresses for the 2 nodes to communicate.
  const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

  uint8_t radio_avaliable(void);
  uint8_t radio_xchange(uint8_t *buf, uint8_t size);
#endif












void setup()
{
  memcpy(command_tx, robo, sizeof(robo) );
   
//  pinMode(led_0, OUTPUT);
//  digitalWrite(led_0, LOW);

  //Для электродвигателей
  pinMode(WAY_MOTOR_LEFT_PIN, OUTPUT);
  pinMode(WAY_MOTOR_RIGHT_PIN, OUTPUT);
  motorPower = DEFAULT_MOTOR_POWER;
  Motor_Left(true, false);
  Motor_Right(true, false);


  //Для дальномера
#if ( HCSR04_CENTER_ENABLE == 1 )
  pinMode(HCSR04_CENTER_TRIG_PIN, OUTPUT); 
  pinMode(HCSR04_CENTER_ECHO_PIN, INPUT); 
  digitalWrite(HCSR04_CENTER_TRIG_PIN, LOW);
#endif

#if ( HCSR04_LEFT_ENABLE == 1 )
  pinMode(HCSR04_LEFT_TRIG_PIN, OUTPUT); 
  pinMode(HCSR04_LEFT_ECHO_PIN, INPUT); 
  digitalWrite(HCSR04_LEFT_TRIG_PIN, LOW);
#endif

#if ( HCSR04_RIGHT_ENABLE == 1 )
  pinMode(HCSR04_RIGHT_TRIG_PIN, OUTPUT); 
  pinMode(HCSR04_RIGHT_ECHO_PIN, INPUT); 
  digitalWrite(HCSR04_RIGHT_TRIG_PIN, LOW);
#endif

  pinMode(DETECTOR_LINE_LEFT_PIN, INPUT); 
  pinMode(DETECTOR_LINE_RIGHT_PIN, INPUT); 


  Serial.begin(115200);


#if (NRF24_INTERFACE_ENABLE == 1)
  printf_begin();  //Отладочная информация по последовательному порту 
#endif


  rx_packet_id = 0;
  rx_packet_num = 0;
  rx_packet_size = 0;
  size_tx = 0;

  sensors.line_left = 0;
  sensors.line_right = 0;
  sensors.wall_left = 3;
  sensors.wall_right = 4;
  sensors.u_sonic_left = 70;
  sensors.u_sonic_center = 70;
  sensors.u_sonic_right = 70;
  sensors.gyro_X = 8;
  sensors.gyro_Y = 9;
  sensors.gyro_Z = 10;
  sensors.photo = 11;
  sensors.thermo = 12;

  RX_state = RX_STATE__wait_R;
  wait_id = 0;



#if (NRF24_INTERFACE_ENABLE == 1)
  radio.begin();
  radio.setChannel(100);
  radio.setRetries(0,15);
  radio.setPayloadSize(32);
  radio.setAutoAck(1);

  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);

  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  radio.printDetails();

  FIFO_FLUSH(fifo_in);
#endif




}






void loop() 
{
  if( (millis() - started_waiting_at) > 2000 )  //Очистим очередь от устаревших команд
  {
#if (NRF24_INTERFACE_ENABLE == 1)
    FIFO_FLUSH(fifo_in)
#else
    Serial.flush();
#endif
  }

#if (NRF24_INTERFACE_ENABLE == 1) 
  radio_xchange(command_tx, size_tx);
#else
  Serial.write(command_tx, size_tx);
#endif
   
  size_tx = 0;
  if ( RX_state != RX_STATE__data_accepted )
  {

#if (NRF24_INTERFACE_ENABLE == 1)    
   while( (! FIFO_IS_EMPTY(fifo_in)) )
    {
      inByte = FIFO_FRONT(fifo_in);
      FIFO_POP(fifo_in);
#else
   while( Serial.available() > 0 )
    {
      inByte = Serial.read();
#endif   
      switch (RX_state)
      {
        case RX_STATE__wait_R :
          if( inByte == 'R')
          {
            RX_state = RX_STATE__wait_O;
          }
          else
          {
            RX_state = RX_STATE__wait_R;
          }
          break;
        case RX_STATE__wait_O :
          if( inByte == 'O')
          {
            RX_state = RX_STATE__wait_B;
          }
          else
          {
            RX_state = RX_STATE__wait_R;
          }
          break;
        case RX_STATE__wait_B :
          if( inByte == 'B')
          {
            RX_state = RX_STATE__wait_O1;
          }
          else
          {
            RX_state = RX_STATE__wait_R;
          }
          break;
        case RX_STATE__wait_O1 :
          if( inByte == 'O')
          {
            RX_state = RX_STATE__wait_i;
          }
          else
          {
            RX_state = RX_STATE__wait_R; 
          }
          break;
        case RX_STATE__wait_i :
          if( inByte == ':')
          {
            RX_state = RX_STATE__wait_num;
          }
          else
          {
            RX_state = RX_STATE__wait_R;     
          }
          break;
        case RX_STATE__wait_num :
          rx_packet_num = inByte;
          RX_state = RX_STATE__wait_size;
          break;
        case RX_STATE__wait_size :
          rx_packet_size = inByte;
          if( rx_packet_size < sizeof(command_rx) )
          {
            RX_state = RX_STATE__wait_data;
            rx_data_count = 0;
          }
          else
          {
            RX_state = RX_STATE__wait_R;
            rx_packet_size = 0;        
          }
          break;
        case RX_STATE__wait_data :
          command_rx[rx_data_count] = inByte;
          RX_state = RX_STATE__wait_data;
          rx_data_count++;
          if( rx_data_count == rx_packet_size )
          {
            RX_state = RX_STATE__data_accepted;
            started_waiting_at = millis();
            //printf("%02X\n", rx_packet_num);
          }
          else
          {
            RX_state = RX_STATE__wait_data;
          }
          break; 
        default :
          RX_state = RX_STATE__wait_R;
      }

     if ( RX_state == RX_STATE__data_accepted)
       break;
    }
  }


  if( RX_state == RX_STATE__data_accepted )
  {
    RX_state = RX_STATE__wait_R;
  
    switch ( command_rx[0] )
    {
      case SCRATCH_TAG__poll :
        break;
      case SCRATCH_TAG__reset_all :
        motorPower = DEFAULT_MOTOR_POWER;
        Motor_Left(true, false);
        Motor_Right(true, false);
       // printf("---\n");
        break;
      case SCRATCH_TAG__move_speed :
        motorPower = from_percent_to_pwm(command_rx[1]);
        break; 
      case SCRATCH_TAG__move_front :
          Motor_Left(true, true);
          Motor_Right(true, true);
          wait_command = 1;
          wait_id = ((unsigned int)(command_rx[1]) << 8) | (unsigned int)(command_rx[2]);
          delay_ms = ((unsigned long )(command_rx[3]) << 24) |
                     ((unsigned long )(command_rx[4]) << 16) |
                     ((unsigned long )(command_rx[5]) << 8) |
                      (unsigned long )(command_rx[6]);
          time_interval_finish = millis() + delay_ms;
         // printf("f\n");
        break;
      case SCRATCH_TAG__move_left :
          Motor_Left(true, true);
          Motor_Right(false, true);  
          wait_command = 1;
          wait_id = ((unsigned int)(command_rx[1]) << 8) | (unsigned int)(command_rx[2]);
          delay_ms = ((unsigned long )(command_rx[3]) << 24) |
                     ((unsigned long )(command_rx[4]) << 16) |
                     ((unsigned long )(command_rx[5]) << 8) |
                      (unsigned long )(command_rx[6]);
          time_interval_finish = millis() + delay_ms;
         // printf("l\n");
        break;
      case SCRATCH_TAG__move_right :
          Motor_Left(false, true);
          Motor_Right(true, true);     
          wait_command = 1;
          wait_id = ((unsigned int)(command_rx[1]) << 8) | (unsigned int)(command_rx[2]);
          delay_ms = ((unsigned long )(command_rx[3]) << 24) |
                     ((unsigned long )(command_rx[4]) << 16) |
                     ((unsigned long )(command_rx[5]) << 8) |
                      (unsigned long )(command_rx[6]);
          time_interval_finish = millis() + delay_ms;
         // printf("r\n");
        break;
      case SCRATCH_TAG__move_back :
          Motor_Left(false, true);
          Motor_Right(false, true);     
          wait_command = 1;
          wait_id = ((unsigned int)(command_rx[1]) << 8) | (unsigned int)(command_rx[2]);
          delay_ms = ((unsigned long )(command_rx[3]) << 24) |
                     ((unsigned long )(command_rx[4]) << 16) |
                     ((unsigned long )(command_rx[5]) << 8) |
                      (unsigned long )(command_rx[6]);
          time_interval_finish = millis() + delay_ms;
         // printf("b\n");
        break;
      case SCRATCH_TAG__servo_0_degrees :
        break;
      case SCRATCH_TAG__servo_1_degrees :
        break;
      case SCRATCH_TAG__servo_2_degrees :
        break;
      case SCRATCH_TAG__servo_3_degrees :
        break;
      case SCRATCH_TAG__led_0 :
        //digitalWrite(led_0, command_rx[2]);
        //printf("led_0 %d\n", command_rx[2]);
        break;
      case SCRATCH_TAG__led_1 :
        //digitalWrite(led_1, command_rx[2]);
        break;
      case SCRATCH_TAG__led_2 :
        //digitalWrite(led_2, command_rx[2]);
        break;
      case SCRATCH_TAG__piezo_tone :
        break;
    //  default :
    }


    command_tx[5] = rx_packet_num;
    if( wait_command == 1 )
    {
      command_tx[6] = 15;
      command_tx[7] = SCRATCH_TAG__move_front; //0 
      command_tx[8] = ( (unsigned char)( (wait_id >> 8 ) & 0xFF) ); //1
      command_tx[9] = ( (unsigned char)( (wait_id      ) & 0xFF) ); //2 

      command_tx[10] = sensors.line_left; //1
      command_tx[11] = sensors.line_right; //2
      command_tx[12] = sensors.wall_left; //3
      command_tx[13] = sensors.wall_right; //4
      command_tx[14] = sensors.u_sonic_left; //5
      command_tx[15] = sensors.u_sonic_center; //6
      command_tx[16] = sensors.u_sonic_right; //7
      command_tx[17] = sensors.gyro_X; //8
      command_tx[18] = sensors.gyro_Y; //9
      command_tx[19] = sensors.gyro_Z; //10
      command_tx[20] = sensors.photo; //11
      command_tx[21] = sensors.thermo; //12
      size_tx = 22;                
    }
    else
    {
      command_tx[6] = 13;
      command_tx[7] = SCRATCH_TAG__poll; //0   

      command_tx[8] = sensors.line_left; //1
      command_tx[9] = sensors.line_right; //2
      command_tx[10] = sensors.wall_left; //3
      command_tx[11] = sensors.wall_right; //4
      command_tx[12] = sensors.u_sonic_left; //5
          //  printf("0x%02X\n", sensors.u_sonic_left);
      command_tx[13] = sensors.u_sonic_center; //6
      command_tx[14] = sensors.u_sonic_right; //7
      command_tx[15] = sensors.gyro_X; //8
      command_tx[16] = sensors.gyro_Y; //9
      command_tx[17] = sensors.gyro_Z; //10
      command_tx[18] = sensors.photo; //11
      command_tx[19] = sensors.thermo; //12 
      size_tx = 20;
    }
  }


  if( wait_command == 1 )
  {
    if( millis() > time_interval_finish ) //Таймаут
    {
      wait_command = 0;

      Motor_Left(true, false);
      Motor_Right(true, false);
    }
  }





  switch ( (rx_packet_num & 0x03) ) //По времени разнесём работу дальномеров. Чтобы друг другу не мешали...
  {
    case 0x00 :
#if ( HCSR04_CENTER_ENABLE == 1 )
    {
       uint8_t distance_sm; 
       digitalWrite(HCSR04_CENTER_TRIG_PIN, HIGH); // Подаем сигнал на выход микроконтроллера 
       delayMicroseconds(10); // Удерживаем 10 микросекунд 
       digitalWrite(HCSR04_CENTER_TRIG_PIN, LOW); // Затем убираем 
       distance_sm = pulseIn(HCSR04_CENTER_ECHO_PIN, HIGH, 10000)/58; // Замеряем длину импульса,// Пересчитываем в сантиметры
       if( (distance_sm < 70) && (distance_sm > 1) )
       {
         sensors.u_sonic_center = distance_sm;
       }
    }
#endif
      break;
    case 0x01 :
#if ( HCSR04_LEFT_ENABLE == 1 )
    { 
       uint8_t distance_sm; 
       digitalWrite(HCSR04_LEFT_TRIG_PIN, HIGH); // Подаем сигнал на выход микроконтроллера 
       delayMicroseconds(10); // Удерживаем 10 микросекунд 
       digitalWrite(HCSR04_LEFT_TRIG_PIN, LOW); // Затем убираем 
       distance_sm = pulseIn(HCSR04_LEFT_ECHO_PIN, HIGH, 10000)/58; // Замеряем длину импульса,// Пересчитываем в сантиметры
       //printf("0x%02X\n", distance_sm);
       if( (distance_sm < 70) && (distance_sm > 1) )
       {
         sensors.u_sonic_left = distance_sm;
       }
     }
#endif
    case 0x02 :
#if ( HCSR04_RIGHT_ENABLE == 1 )
    { 
       uint8_t distance_sm; 
       digitalWrite(HCSR04_RIGHT_TRIG_PIN, HIGH); // Подаем сигнал на выход микроконтроллера 
       delayMicroseconds(10); // Удерживаем 10 микросекунд 
       digitalWrite(HCSR04_RIGHT_TRIG_PIN, LOW); // Затем убираем 
       distance_sm = pulseIn(HCSR04_RIGHT_ECHO_PIN, HIGH, 10000)/58; // Замеряем длину импульса,// Пересчитываем в сантиметры
       if( (distance_sm < 70) && (distance_sm > 1) )
       {
         sensors.u_sonic_right = distance_sm;
       }
     }
#endif    
      break;

    case 0x03 :
      break;
}






{
  sensors.line_left = digitalRead(DETECTOR_LINE_LEFT_PIN);
  sensors.line_right = digitalRead(DETECTOR_LINE_RIGHT_PIN);
}









}







#if (NRF24_INTERFACE_ENABLE == 1)
uint8_t radio_xchange(uint8_t *buf, uint8_t size) //!!! size < 29 !!!
{
  while(1)
  {
    if ( radio.available() )
    {
      bool done = false;
      while (!done)
      {
        done = radio.read( Buf_From_RF24.Buf, 32 );
      }
      Buf_To_RF24.Buf[0] = Buf_From_RF24.Buf[0];
      Buf_To_RF24.Buf[2] = size;
      //printf("=%d\n", size);
      memcpy(&Buf_To_RF24.Buf[3], buf, size);     
      radio.stopListening();
      radio.write( Buf_To_RF24.Buf, 32 );
      radio.startListening();    
      if(Buf_From_RF24.Buf[2] > 0)
      {
        for(int i = 0; i < Buf_From_RF24.Buf[2]; i++)
        {
          FIFO_PUSH(fifo_in, Buf_From_RF24.Buf[i + 3]);
        }
      }
      break; 
    }
  }
  return FIFO_COUNT(fifo_in);
}
#endif


void Motor_Left(bool direct, bool enable)
{
  if(enable)
  {
    if (direct)
    {
      analogWrite(PWM_MOTOR_LEFT_PIN, (255 - motorPower));
      digitalWrite(WAY_MOTOR_LEFT_PIN, HIGH);
    }
    else
    {
      analogWrite(PWM_MOTOR_LEFT_PIN, motorPower);
      digitalWrite(WAY_MOTOR_LEFT_PIN, LOW);
    }
  }
  else
  {
    analogWrite(PWM_MOTOR_LEFT_PIN, 0);
    digitalWrite(WAY_MOTOR_LEFT_PIN, LOW);
  }
}


void Motor_Right(bool direct, bool enable)
{
  if(enable)
  {
    if(direct)
    {      
      analogWrite(PWM_MOTOR_RIGHT_PIN, (255 - motorPower));
      digitalWrite(WAY_MOTOR_RIGHT_PIN, HIGH);
    }
    else
    {   
      analogWrite(PWM_MOTOR_RIGHT_PIN, motorPower);
      digitalWrite(WAY_MOTOR_RIGHT_PIN, LOW);
    }
  }
  else
  {
    analogWrite(PWM_MOTOR_RIGHT_PIN, 0);
    digitalWrite(WAY_MOTOR_RIGHT_PIN, LOW);
  }
}


uint8_t from_percent_to_pwm(uint8_t percent)
{  
  float pwm  = (percent > 100) ? 100 : percent;
  pwm = 255 * (pwm/100);

  return (uint8_t)(pwm);
}







