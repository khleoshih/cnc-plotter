#include <AccelStepper.h>
#include <Stepper.h>
#include <Servo.h>

#define HALFSTEP 4
#define STEPS_PER_MOTOR_REVOLUTION 32   
#define STEPS_PER_OUTPUT_REVOLUTION 32 * 64  //2048
#define MM_TO_STEPS 200
#define PEN_UP_ANGLE 60
#define PEN_DOWN_ANGLE 110
#define OPERATION_DELAY 50

#define motorXpin1 10
#define motorXpin2 11
#define motorXpin3 12
#define motorXpin4 13

#define motorYpin1 3
#define motorYpin2 4
#define motorYpin3 5
#define motorYpin4 6

#define servopin 2


//AccelStepper stepperX(HALFSTEP, motorXpin1, motorXpin3, motorXpin2, motorXpin4);
//AccelStepper stepperY(HALFSTEP, motorYpin1, motorYpin3, motorYpin2, motorYpin4);
Stepper stepperX(STEPS_PER_MOTOR_REVOLUTION,  motorXpin1, motorXpin3, motorXpin2, motorXpin4);
Stepper stepperY(STEPS_PER_MOTOR_REVOLUTION,  motorYpin1, motorYpin3, motorYpin2, motorYpin4);
Servo penservo;
int posX, posY, targetX, targetY, state;
int index;
char line[ 512 ];
String pos;
void setup() {
  //stepperX.setMaxSpeed(1000.0);
  //stepperX.setAcceleration(100.0);
  //stepperY.setMaxSpeed(1000.0);
  //stepperY.setAcceleration(100.0);
  penservo.attach(servopin);
  targetX = 0;
  targetY = 0;
  posX = 0;
  posY = 0;
  state = PEN_UP_ANGLE;
  index = 0;
  penservo.write(state);
  stepperX.setSpeed(500);
  stepperY.setSpeed(500);
  Serial.begin(9600);
}//--(end setup )---

void loop() {
  char temp;
  //posX = stepperX.currentPosition();
  //posY = stepperY.currentPosition();
  //pos = String(posX) + ',' + String(posY);
  //Serial.println(pos);
  while (Serial.available() > 0) {
    //int message = Serial.parseInt();
    temp = Serial.read();
    if (temp == 'x'){
      line[index] = '\0';
      targetX = atoi(line);
      stepperX.step(targetX);
      posX += targetX;
      transmit_pos();
      targetX = 0;
      index = 0;
    }
    else if (temp == 'y'){
      line[index] = '\0';
      targetY = atoi(line);  
      stepperY.step(targetY);
      posY += targetY;
      transmit_pos();
      targetY = 0;
      index = 0;
    }
    else if (temp == 'p'){
      line[index] = '\0';
      state = atoi(line);
      penservo.write(state);
      delay(OPERATION_DELAY);
      index = 0;
    }
    else if (temp == 'g'){
      line[index] = '\0';
      Gcode();
      transmit_pos();
      Serial.println("GcodeDone");
      index = 0;
    }
    else if (temp == 'o'){
      posX = 0;
      posY = 0;
      transmit_pos();
      index = 0;
    }
    else{
      if (temp <= ' '){}
      else if (temp == '/'){}
      else{
        line[index++] = temp;
      }
    }
  }
  //stepperX.moveTo(targetX);
  //stepperX.setSpeed(1000);
  //stepperX.runSpeedToPosition();
  //stepperY.moveTo(targetY);
  //stepperY.setSpeed(1000);
  //stepperY.runSpeedToPosition();
}

void Gcode(){
  //Serial.println(line);
  switch(line[0]){
    case 'G':
      if (line[1] == '0'){
        char* X = strchr(line, 'X');
        char* Y = strchr(line, 'Y');
        if ((X != NULL) && (Y != NULL)){
          float xval = atof(X+1);
          float yval = atof(Y+1);
          Move((int)(xval*MM_TO_STEPS), (int)(yval*MM_TO_STEPS));
        }       
      }
      break;
    case 'M':
      if ((line[1] == '0') && (line[2] == '3')){
        state = PEN_DOWN_ANGLE;
        penservo.write(state);
        delay(OPERATION_DELAY);
      }
      else if ((line[1] == '0') && (line[2] == '5')){
        state = PEN_UP_ANGLE;
        penservo.write(state);
        delay(OPERATION_DELAY);
      }
      break;
    default:
      break;
  }
}

void Move(int tar_x, int tar_y){
  int dx, dy;
  int over = 0;
  //Serial.println("X target:");
  //Serial.println(tar_x);
  //Serial.println("Y target:");
  //Serial.println(tar_y);
  targetX = tar_x - posX;
  targetY = tar_y - posY;
  if (targetX > 0) dx = 1;
  else dx = -1;
  if (targetY > 0) dy = 1;
  else dy = -1; 
  if (abs(targetX) > abs(targetY)){
    for (int i = 0; i < abs(targetX); ++i){
      stepperX.step(dx);
      posX += dx;
      over += abs(targetY);
      if (over >= abs(targetX)){
        over -= abs(targetX);
        stepperY.step(dy);
        posY += dy;
      }
    }
  }
  else{
    for (int i = 0; i < abs(targetY); ++i){
      stepperY.step(dy);
      posY += dy;
      over += abs(targetX);
      if (over >= abs(targetY)){
        over -= abs(targetY);
        stepperX.step(dx);
        posX += dx;
      }
    }
  }
  //Serial.println("X pos:");
  //Serial.println(posX);
  //Serial.println("Y pos:");
  //Serial.println(posY);
  //posX = tar_x;
  //posY = tar_y;
}

void transmit_pos(){
  pos = "Position:" + String(posX) + "," + String(posY);
  Serial.println(pos);
}
