#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "MeMCore.h"

MeDCMotor MotorL(M1);
MeDCMotor MotorR(M2);
MeUltrasonicSensor ultr(PORT_1);
MeLineFollower line(PORT_3);
Me7SegmentDisplay ecr(PORT_2);
int moveSpeed = 120;
int LineFollowFlag;
int compteurG = 0;
int counterObs = 0;
long time1 = 0;
int correctionR = 0;
int correctionL = 0;
long timeVirAller = 0;
int virAllerOK = 0;
int checkDM = 0;
long tempsecran = 0;
bool state= true;




void Forward()
{
  MotorL.run(-moveSpeed);
  MotorR.run(moveSpeed);
}
void Backward(int temps)
{
  MotorL.run(moveSpeed);
  MotorR.run(-moveSpeed);
  delay(temps);
}
void TurnLeft(int temps)
{
  MotorL.run(moveSpeed);
  MotorR.run(moveSpeed);
  delay(temps);
}
void TurnRight(int temps)
{
  MotorL.run(-moveSpeed);
  MotorR.run(-moveSpeed);
  delay(temps);
}

int DemiTour(int compteurL) {
  digitalWrite(13,HIGH);
  if (compteurL == 0) {
    TurnRight(200);
    uint8_t val;
    val = line.readSensors();
    while(val !=S1_IN_S2_IN){
      TurnRight(0);
      val = line.readSensors();
    }
    compteurL++;
  }
  else {
    while(1){
      MotorL.stop();
      MotorR.stop();
    }
  }
  
  return compteurL;
}

int CheckRight() {
  int detect = 0;
  for (int i = 1; i < 7; i++) {
    TurnRight(100);
    uint8_t val = line.readSensors();
    if (val == S1_IN_S2_IN) {
      detect = 1;
      break;
    }
  }
  return detect;
}

int CheckLeft() {
  TurnLeft(750);
  int detect = 0;
  for (int j = 1; j < 6; j++) {
    TurnLeft(100);
    uint8_t val = line.readSensors();
    if (val == S1_IN_S2_IN) {
      detect = 1;
      break;
    }
  }
  return detect;
}

void dispObs() {
  if (-tempsecran + millis() > 3000){
    state = !state;
    tempsecran= millis();
    }
  if (state==true){
    ecr.display(counterObs);
    
  }
  else{
    ecr.display(int(millis()/1000));
  }
}

void readSensor() {
  float pult = (float)ultr.distanceCm(50000);
  if (pult < 7 && time1 + 3000 < millis()) {
    counterObs++;
    time1 = millis();
  }
  while (pult < 7) {
    avoidObstacle();
    pult = (float)ultr.distanceCm(50000);
  }
}



void avoidObstacle() {
  int detectR = CheckRight();
  if (detectR == 0) {
    int detectL = CheckLeft();
    if (detectL == 0) {
      TurnLeft(500);
    }
  }
  dispObs();
}


void dirChange(uint8_t value) {
  switch (value)
  {
    case S1_IN_S2_IN:
      Forward();
      LineFollowFlag = 10;
      break;

    case S1_IN_S2_OUT:
      Forward();
      //if (LineFollowFlag > 1) LineFollowFlag--;
      TurnLeft(40);
      break;

    case S1_OUT_S2_IN:
      Forward();
      //if (LineFollowFlag < 20) LineFollowFlag++;
      TurnRight(40);
      break;

    case S1_OUT_S2_OUT:
      if (LineFollowFlag < 10) {
        TurnLeft(40);
        correctionL++;
      }
      

      if (LineFollowFlag > 10 ) {
        TurnRight(40);
        correctionR++;
      }

      if (LineFollowFlag == 10){
          //compteurG = DemiTour(compteurG);
          checkDemiTour();
          if (checkDM == 1) compteurG = DemiTour(compteurG);
        } 
      break;
  }
}


int checkDemiTour(){
    Backward(50);
    uint8_t val;
    for (int i=0; i<4; i++){
        TurnRight(200);
        val = line.readSensors();
        if (val != S1_OUT_S2_OUT) return 0;       
      }
      TurnLeft(600);
     for (int j=0; j<4; j++){
        TurnLeft(200);
        val = line.readSensors();
        if (val != S1_OUT_S2_OUT) return 0;      
      }
    checkDM = 1;

  }



void lookForRouteR(){
  uint8_t val;
  val = line.readSensors();
  while (val != S1_IN_S2_OUT){
    TurnRight(0);
    val = line.readSensors();
  }
  val = line.readSensors();
  while (val!=S1_IN_S2_IN){
    Forward();
    val = line.readSensors();
    if (val ==S1_OUT_S2_OUT){
      TurnLeft(0);
    }
  }
  Forward();
  delay(400);
  TurnRight(675);
  virAllerOK++;
 }

 void lookForRouteL(){
  
  uint8_t val;
  val = line.readSensors();
  while(val!=S1_IN_S2_OUT){
    Forward();
    val = line.readSensors();
  }

  val = line.readSensors();
  while (val != S1_OUT_S2_IN){
    TurnLeft(0);
    val = line.readSensors();
  }
  val = line.readSensors();
  while (val!=S1_IN_S2_IN){
    Forward();
    val = line.readSensors();
    if (val ==S1_OUT_S2_OUT){
      TurnRight(0);
    }
    
  }
  
  Forward();
  delay(400);
  TurnLeft(675);
  virAllerOK++;
 }



void modeC()
{
  uint8_t val;
  val = line.readSensors();
  dirChange(val);
}




void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop() {
  modeC();
  readSensor();
  dispObs();
  if(millis() >= 38000 && millis() <= 38700) TurnRight(0);
  if(virAllerOK==0 && millis()>52000) lookForRouteR();
  if(compteurG==1 && virAllerOK==1){
    digitalWrite(13,LOW);
    lookForRouteL();
  }

}
