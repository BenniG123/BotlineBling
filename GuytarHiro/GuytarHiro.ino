#include <Servo.h>

Servo Strum, StarPower;

int incomingByte;
int StrumUp = 143;
int StrumDown = 38;
int StarPress = 45;
int StarIdle = 90;
int UpFlag = 0;
int DownFlag = 1;

const int Green =  13, Red =  12, Yellow =  11, Blue =  10, Orange =  9;

void setup() {

  pinMode(Green, OUTPUT);
  pinMode(Red, OUTPUT);
  pinMode(Yellow, OUTPUT);
  pinMode(Blue, OUTPUT);
  pinMode(Orange, OUTPUT);

  Strum.attach(6);
  StarPower.attach(5);

  Strum.write(90);
  
  Serial.begin(9600);
}

void loop() {
  
  if (Serial.available() > 0) {
    
    incomingByte = Serial.read();
    
    
    if(incomingByte & 0x01) {
      digitalWrite(Green, HIGH);
    } else {
      digitalWrite(Green, LOW);
    }
    
    if(incomingByte & 0x02) {
      digitalWrite(Red, HIGH);
    } else {
      digitalWrite(Red, LOW);
    }
    
    if(incomingByte & 0x04) {
       digitalWrite(Yellow, HIGH);
    } else {
      digitalWrite(Yellow, LOW);
    }
    
    if(incomingByte & 0x08) {
      digitalWrite(Blue, HIGH);
    } else {
      digitalWrite(Blue, LOW);
    }

    if(incomingByte & 0x10) {
      digitalWrite(Orange, HIGH);
    } else {
      digitalWrite(Orange, LOW);
    }

    if(incomingByte & 0x20 && DownFlag) { // Strum
      Strum.write(StrumUp);
      DownFlag = 0;
      UpFlag = 1;
    } else if(incomingByte & 0x20 && UpFlag) { // Strum 
      Strum.write(StrumDown);
      DownFlag = 1;
      UpFlag = 0;
    } else {
      //Strum.write(StrumIdle);
    }

    if(incomingByte & 0x40) { // Star Power
      StarPower.write(StarPress);
    } else {
      StarPower.write(StarIdle);
    }     
  }
}    
