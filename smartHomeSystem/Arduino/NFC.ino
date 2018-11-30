#include <SoftwareSerial.h> // 라즈베리파이와 블루투스 통신을 하기 위한 헤더파일

#define RETURN_WAKE_SIZE 15
#define RETURN_FIRMWARE_SIZE 19
#define RETURN_TAG_SIZE 25

#define PIN_DOOR 8
#define PIN_NFC_RX 0
#define PIN_NFC_TX 1
 
const unsigned char wake[24]={
  0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfd, 0xd4, 0x14, 0x01, 0x17, 0x00};//wake up NFC module
const unsigned char firmware[9]={
  0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};//
const unsigned char tag[11]={
  0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};//detecting tag command
const unsigned char std_ACK[25] = {
  0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x0C, \
0xF4, 0xD5, 0x4B, 0x01, 0x01, 0x00, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00};

unsigned char old_id[5];
unsigned char cur_id[5];

unsigned char receive_ACK[25];//Command receiving buffer
unsigned char test_nfc_tag[25] = {
 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x0C, \
0xF4, 0xD5, 0x4B, 0x01, 0x01, 0x00, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00};

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define print1Byte(args) Serial1.write(args)
#define print1lnByte(args)  Serial1.write(args),Serial1.println()
#else
#include "WProgram.h"
#define print1Byte(args) Serial1.print(args,BYTE)
#define print1lnByte(args)  Serial1.println(args,BYTE)
#endif


bool is_close = true;
char flag_open;

///////////////////// 블루투스 통신을 위한 설정
int bluetoothTx = 2;
int bluetoothRx = 3;

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);


void setup()
{
  Serial.begin(9600);     // open serial with PC
  //while(! Serial); // 시리얼 모니터가 실행될 때까지 대기
  
  Serial.println("Start serial monitor");
  
  Serial1.begin(115200);  // open serial with NFC module
  bluetooth.begin(9600); // open serial with ras
  delay(100);
  
  NFC_init();

  pinMode(PIN_DOOR, OUTPUT);
  
  /*
  pinMode(power_Relay,OUTPUT);
  pinMode(door_Relay,OUTPUT);
  pinMode(button,INPUT_PULLUP);
  */
}
 
void loop()
{
  Serial.println("in loop()");
  while(is_close)
  {
    send_tag(); 
    read_ACK(25);
    delay(100);
    
    if (!cmp_id()) // 이전 id와 입력으로 들어온 id가 다르다면
    {
      if (test_ACK()) // ACK가 원하는 형태라면
      {
        display(RETURN_TAG_SIZE);
        delay(100);
        copy_id();
        cmp_tag();      
      }
    }
  }

  Serial.println("out while");
  delay(5000);
  is_close = true;
  //flag_open = 'r';
  
  /*
  checking();
 
  //Sleep Mode
  attachInterrupt(0, wakeUp, LOW);  
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //set Sleep mode
  detachInterrupt(0);    
  */
  
}

void NFC_init(){
    Serial.println("in NFC_init()");
    delay(100);  // give delay before sending command bytes
    wake_card();
    delay(100);
    read_ACK(RETURN_WAKE_SIZE);
    delay(100);
    display(RETURN_WAKE_SIZE);
     
    firmware_version();
    delay(100);
    read_ACK(RETURN_FIRMWARE_SIZE);
    delay(100);
    display(RETURN_FIRMWARE_SIZE);
}
 
void UART_Send_Byte(unsigned char command_data) // send byte to PC
{
  Serial.print(command_data,HEX);
  Serial.print(" ");
} 

void UART1_Send_Byte(unsigned char command_data) // send byte to device
{
  print1Byte(command_data);
#if defined(ARDUINO) && ARDUINO >= 100
  Serial1.flush();// complete the transmission of outgoing serial data 
#endif
}

void read_ACK(unsigned char temp) // read ACK into reveive_ACK[]
{
  unsigned char i;
  for(i = 0; i < temp; i++)
    receive_ACK[i]= Serial1.read();
}
 
void wake_card(void) // send wake[] to device
{
  unsigned char i;
  for(i = 0; i < 24; i++) // send command
    UART1_Send_Byte(wake[i]);
}
 
void firmware_version(void) // send firmware[] to device
{
  unsigned char i;
  for(i = 0; i < 9; i++) // send command
    UART1_Send_Byte(firmware[i]);
}
 
void send_tag(void) // send tag[] to device
{
  unsigned char i;
  for(i = 0; i < 11; i++) // send command
    UART1_Send_Byte(tag[i]);
}

void copy_id (void) {//save old id
  int ai, oi;
  for (oi = 0, ai = 19; oi < 5; oi++,ai++) {
    old_id[oi] = receive_ACK[ai];
  }
}
 
int cmp_id (void) // return true if find id is old
{
  int ai, oi;
  for (oi = 0, ai = 19; oi < 5; oi++,ai++) {
    if ((old_id[oi] != receive_ACK[ai]) && (receive_ACK[ai] != 0xFF))
      return 0;
  }
  return 1;
}

int test_ACK (void) // return true if receive_ACK accord with std_ACK
{
  int i;
  for (i = 0; i < 5; i++) {
    if (receive_ACK[i] != std_ACK[i])
      return 0;
  }
  return 1;
}
 
void display(unsigned char tem)
{
  for (int i = 0; i < tem; i++)
  {
    if (receive_ACK[i] < 16) // to make 2 digits
      Serial.print("0");
    Serial.print(receive_ACK[i], HEX);
    if (i < (tem-1)) Serial.print(" ");
  }
  Serial.println();
}

void cmp_tag()
{
  if(old_id[0] == 0x79 && old_id[1] == 0xBE && old_id[2] == 0x5B && old_id[3] == 0xE5)
    door_open();
  else if(old_id[0] == 0x04 && old_id[1] == 0x67 && old_id[2] == 0x20 && old_id[3] == 0x92)
    door_open();
  else if(old_id[0] == 0x04 && old_id[1] == 0x74 && old_id[2] == 0xBD && old_id[3] == 0x92)
    door_open();
  else// if(flag_open == 'r')
    set_notification('x');
}

void door_open()
{
  // ON C와 NO가 연결됨
  digitalWrite(PIN_DOOR, HIGH);
  delay(500);
  digitalWrite(PIN_DOOR, LOW);
  
  
  is_close = false; // loop의 while문을 잠시 멈추기 위한 플래그
  
  set_notification('o');
}

void set_notification(char data)
{
  flag_open = data;
  bluetooth.print(flag_open);

  clear_tag();
}

void clear_tag()
{
  old_id[0] = 0x00;
  old_id[1] = 0x00;
} 

