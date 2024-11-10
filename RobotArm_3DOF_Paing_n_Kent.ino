//Written by Paing Thet Ko
//Debugged by Kent
//Sponsered by Thura Zaw

#include <stdio.h>
//Includes the Arduino Stepper Library

//RAMPS 1.4 PINS
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
// #define X_MAX_PIN           2
 
#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define E0_STEP_PIN        26
#define E0_DIR_PIN         28
#define E0_ENABLE_PIN      24
#define E0_MIN_PIN         20

#define E1_STEP_PIN        36
#define E1_DIR_PIN         34
#define E1_ENABLE_PIN      30

#define BYJ_PIN_0          40
#define BYJ_PIN_1          63
#define BYJ_PIN_2          59
#define BYJ_PIN_3          64

#define SERVO_PIN           4

#define PUMP_PIN            8
#define LASER_PIN          10
#define LED_PIN            13

#define SDPOWER            -1
#define SDSS               53

#define FAN_PIN             9

#define PS_ON_PIN          12
#define KILL_PIN           -1

//#define HEATER_0_PIN       10
//#define HEATER_1_PIN        8
#define TEMP_0_PIN         13   // ANALOG NUMBERING
#define TEMP_1_PIN         14   // ANALOG NUMBERING



float previousDeg[3] = {0,0,0} ; // for home position recalculation
float calDeg[3]={0,0,0};

// Stepper 

// Define the number of steps per revolution
const int stepsPerRevolution = 2038;

// Define the delay between steps
const int stepDelay = 3;



struct Motor {
  int stepPin;
  int dirPin;
  int enPin;
  int switchPin;
  float limitAngle;
};

Motor motor[3];

int pulseWidthMicros = 100;  // microseconds
int millisBtwnSteps = 700;
int X = 0; // X motor
int Z = 1; // Z motor  
int Y = 2; // Y motor



void setup() {
  Serial.begin(9600);
  // Bluetooth

  // Stepper 

  // Set the motor pins as outputs
  pinMode(BYJ_PIN_0, OUTPUT);
  pinMode(BYJ_PIN_1, OUTPUT);
  pinMode(BYJ_PIN_2, OUTPUT);
  pinMode(BYJ_PIN_3, OUTPUT);

  // Stepper End 
  motor[0].stepPin = X_STEP_PIN;
  motor[0].dirPin = X_DIR_PIN;
  motor[0].enPin = X_ENABLE_PIN;
  motor[0].switchPin = X_MIN_PIN;
//  motor[0].limitAngle = 55;
  
 
  motor[1].stepPin = Z_STEP_PIN;
  motor[1].dirPin = Z_DIR_PIN;
  motor[1].enPin = Z_ENABLE_PIN;
  motor[1].switchPin = Z_MIN_PIN;
 // motor[1].limitAngle = 60;

  motor[2].stepPin =  Y_STEP_PIN;
  motor[2].dirPin = Y_DIR_PIN;
  motor[2].enPin = Y_ENABLE_PIN;
  motor[2].switchPin = Y_MIN_PIN;
 // motor[2].limitAngle = 180;

  for (int i = 0; i < 3; i++) {
    pinMode(motor[i].enPin, OUTPUT);
    pinMode(motor[i].dirPin, OUTPUT);
    pinMode(motor[i].switchPin, INPUT_PULLUP); // Switch pin state 1
    pinMode(motor[i].stepPin, OUTPUT);
    digitalWrite(motor[i].enPin, LOW); // to stop moving by external factors
  }

  Serial.println("RAMPS Shield Initialized");
  calibrate(); // Go to the end stops
  home(); // Go to home position


  

  // grip();
  // // pick(20,X);
  // release();
  // release();
  // grip();
   
  
}


void loop() {
  readBluetooth();
  //grip();
  //  if (Serial.available()) {
  //   // Read the input string until a newline character is received
  //   String input = Serial.readStringUntil('\n');
  //   Serial.println(input);
  //   // Parse the input string and extract the values
  //   float theta = input.substring(0, input.indexOf(' ')).toFloat();
  //   String motorSuffix = input.substring(input.indexOf(' ') + 1);
    
  //     // Convert the input value to an integer based on the characters X, Y, and Z
  //   int motorNum;
  //   if (motorSuffix == "X") {
  //     motorNum = 0;
  //   } else if (motorSuffix == "Y") {
  //     motorNum = 1;
  //   } else if (motorSuffix == "Z") {
  //     motorNum = 2;
  //   } else {
  //     Serial.println("Invalid input");
  //     return;  // Exit the loop if the input is invalid
  //   }
    
  //   // Call the pick function with the extracted values
  //   bool result = pick(theta, motorNum);
    
  //   // Print the result
  //   Serial.println(result ? "Pick successful" : "Pick failed");
 
  // }
}

void pick(float theta, int motorNum)
{
      float theta1[3];
      theta1[motorNum] = theta;
      Serial.print(motorNum);
      Serial.print("  ");
      //float calDeg[3];
      int steps;

      calDeg[motorNum] =  theta1[motorNum]  - previousDeg[motorNum];
      calDeg[motorNum] > 0 ? digitalWrite(motor[motorNum].dirPin, HIGH) : digitalWrite(motor[motorNum].dirPin, LOW);
      steps = stepConversion(calDeg[motorNum]);
      Serial.println(steps);

      // Calculation of Maximum steps from Limit angle
      // int limitSteps[3];
      // limitSteps[motorNum] = stepConversion(motor[motorNum].limitAngle);
      digitalWrite(motor[motorNum].enPin, LOW);
      for(int i =0; i < abs(steps); i++)
      {
       
          // if(digitalRead(motor[motorNum].switchPin) && i != abs(limitSteps[motorNum]))
          if(digitalRead(motor[motorNum].switchPin))
          {
            digitalWrite(motor[motorNum].stepPin, LOW);
            delayMicroseconds(pulseWidthMicros);
            digitalWrite(motor[motorNum].stepPin, HIGH);
            delayMicroseconds(millisBtwnSteps);
          }
          else {
            digitalWrite(motor[motorNum].enPin, LOW);
            Serial.print("Mininum or Maximum Position of rotation reached for motor ( 0 = X, 1 = Z, 2 = Y) ______");
            Serial.println(motorNum);
          }
       
      }
      digitalWrite(motor[motorNum].enPin, LOW);
      
      previousDeg[motorNum] = calDeg[motorNum]+previousDeg[motorNum]; // Relative position calculation 
      // return true; // Just for future case
}


int stepConversion(float stepDeg)
{
  return stepDeg*40; // ALL resolution pin (M1,M2, M3) high 1/16 step resolution

}


void home(){
  Serial.println("Home Running");
  int stepsX = 1680; //50 degree (1 degree == 40 steps)
  int stepsZ = 720;
  int stepsY = 3600;

  // Rotating the X step motor
  digitalWrite(motor[0].dirPin, HIGH); 
  digitalWrite(motor[0].enPin, LOW);
  for(int i =0; i < stepsX; i++)
  {
      digitalWrite(motor[0].stepPin, LOW);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(motor[0].stepPin, HIGH);
      delayMicroseconds(millisBtwnSteps);
  }

  digitalWrite(motor[1].dirPin, HIGH); 
  digitalWrite(motor[1].enPin, LOW);
  for(int i =0; i < stepsZ; i++)
  {
      digitalWrite(motor[1].stepPin, LOW);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(motor[1].stepPin, HIGH);
      delayMicroseconds(millisBtwnSteps);
  }
 
  digitalWrite(motor[2].dirPin, HIGH); 
  digitalWrite(motor[2].enPin, LOW);
  for(int i =0; i < stepsY; i++)
  {
      digitalWrite(motor[2].stepPin, LOW);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(motor[2].stepPin, HIGH);
      delayMicroseconds(millisBtwnSteps);
  }
 for(int i=0;i<3;i++){
   previousDeg[i] = 0; // for home position recalculation
   calDeg[i]=0;
 }
}

void calibrate() 
{
  digitalWrite(motor[0].enPin, HIGH);
  digitalWrite(motor[1].enPin, HIGH);
  digitalWrite(motor[2].enPin, HIGH);
  
 for(int i = 0; i<3; i++)
  {
      digitalWrite(motor[i].dirPin, LOW); 
      digitalWrite(motor[i].enPin, LOW);
      
     while(digitalRead(motor[i].switchPin))
      {
            digitalWrite(motor[i].stepPin, HIGH);
            delayMicroseconds(pulseWidthMicros);
            digitalWrite(motor[i].stepPin, LOW);
            delayMicroseconds(millisBtwnSteps);
    
      }
     digitalWrite(motor[i].enPin, HIGH);
  }
    digitalWrite(motor[0].enPin, LOW);
    digitalWrite(motor[1].enPin, LOW);
    digitalWrite(motor[2].enPin, LOW);
}


void stepMotor(int currentStep) {
  // Set the motor coil configuration for the current step
  switch (currentStep % 8) {
    case 0:
      digitalWrite(BYJ_PIN_0, HIGH);
      digitalWrite(BYJ_PIN_1, LOW);
      digitalWrite(BYJ_PIN_2, LOW);
      digitalWrite(BYJ_PIN_3, LOW);
      break;
    case 1:
      digitalWrite(BYJ_PIN_0, HIGH);
      digitalWrite(BYJ_PIN_1, HIGH);
      digitalWrite(BYJ_PIN_2, LOW);
      digitalWrite(BYJ_PIN_3, LOW);
      break;
    case 2:
      digitalWrite(BYJ_PIN_0, LOW);
      digitalWrite(BYJ_PIN_1, HIGH);
      digitalWrite(BYJ_PIN_2, LOW);
      digitalWrite(BYJ_PIN_3, LOW);
      break;
    case 3:
      digitalWrite(BYJ_PIN_0, LOW);
      digitalWrite(BYJ_PIN_1, HIGH);
      digitalWrite(BYJ_PIN_2, HIGH);
      digitalWrite(BYJ_PIN_3, LOW);
      break;
    case 4:
      digitalWrite(BYJ_PIN_0, LOW);
      digitalWrite(BYJ_PIN_1, LOW);
      digitalWrite(BYJ_PIN_2, HIGH);
      digitalWrite(BYJ_PIN_3, LOW);
      break;
    case 5:
      digitalWrite(BYJ_PIN_0, LOW);
      digitalWrite(BYJ_PIN_1, LOW);
      digitalWrite(BYJ_PIN_2, HIGH);
      digitalWrite(BYJ_PIN_3, HIGH);
      break;
    case 6:
      digitalWrite(BYJ_PIN_0, LOW);
      digitalWrite(BYJ_PIN_1, LOW);
      digitalWrite(BYJ_PIN_2, LOW);
      digitalWrite(BYJ_PIN_3, HIGH);
      break;
    case 7:
      digitalWrite(BYJ_PIN_0, HIGH);
      digitalWrite(BYJ_PIN_1, LOW);
      digitalWrite(BYJ_PIN_2, LOW);
      digitalWrite(BYJ_PIN_3, HIGH);
      break;
  }
}

void grip() {
  int stepsToMove = (stepsPerRevolution / 12) * 8;  // Rotate half revolution (180 degrees)
  int currentStep = 0;
  
  for (int i = 0; i < stepsToMove; i++) {
    stepMotor(currentStep);
    currentStep++;
    
    if (currentStep >= 8) {
      currentStep = 0;
    }
    
    delay(stepDelay);
  }

  // Stop motor by setting all pins LOW
  digitalWrite(BYJ_PIN_0, LOW);
  digitalWrite(BYJ_PIN_1, LOW);
  digitalWrite(BYJ_PIN_2, LOW);
  digitalWrite(BYJ_PIN_3, LOW);
}


void release() {
  int stepsToMove = (stepsPerRevolution / 14) * 8;  // Rotate half revolution (180 degrees)
  int currentStep = 0;
  
  for (int i = 0; i < stepsToMove; i++) {
    stepMotor(currentStep);
    currentStep--;
    
    if (currentStep < 0) {
      currentStep = 7;
    }
    
    delay(stepDelay);
  }

  // Stop motor by setting all pins LOW
  digitalWrite(BYJ_PIN_0, LOW);
  digitalWrite(BYJ_PIN_1, LOW);
  digitalWrite(BYJ_PIN_2, LOW);
  digitalWrite(BYJ_PIN_3, LOW);
}

void FromPos1(){
  pick(30,Z);
  pick(30,X);
  pick(60,Z);
  pick(60,X);
  pick(85,Z);
  pick(85,X);
}
void ToPos1(){
  pick(60,X);
  pick(60,Z);
  pick(30,X);
  pick(30,Z);
  pick(0,X);
  pick(0,Z); 
  delay(500);
  pick(90,Y);
  pick(10,Z);
  pick(30,X);
  pick(20,Z);
  pick(60,X);
  pick(55,Z);
  pick(95,X);
}
void FromPos2(){
  
  pick(10,Z);
  pick(30,X);
  pick(20,Z);
  pick(60,X);
  pick(55,Z);
  pick(95,X);
}
void ToPos2(){
  pick(60,X);
  pick(20,Z);
  pick(30,X);
  pick(10,Z);
  pick(0,X);
  pick(0,Z);
  delay(500);
  pick(90,Y);
  pick(30,Z);
  pick(30,X);
  pick(60,Z);
  pick(60,X);
  pick(85,Z);
  pick(85,X);


}

void readBluetooth(){

  if (Serial.available()) {
    // Read data from Bluetooth module
    char data = Serial.read();
    switch(data)
    {
      case '0':
      calibrate();
      home();
      break;
      case 'A':
      grip();
      break;
      case 'B':
      release();
      break;
      case '1':
      FromPos1();
      break;
      case '2':
      FromPos2();
      break;
      case '3':
      ToPos1();
      break;
      case '4':
      ToPos2();
      break;
    }
    // Print the received data to the Serial Monitor
    Serial.print("Received: ");
    Serial.println(data);

    // Add your custom logic here to process the received data
  }
}