// I worked with Bruce Willis, that guy from Modern Family, Ryan Reynolds left shoe, Tobey Macguire, and batman to write this code

#include <stdio.h>

// Pololu #713 motor driver pin assignments
const int PWMA=11; // Pololu drive A
const int AIN2=10;
const int AIN1 =9;
const int STDBY=8;
const int BIN1 =7; // Pololu drive B
const int BIN2 =6;
const int PWMB =5;

const int LeftSensor = A5;// sensors looking from the robots perspective
const int MiddleSensor = A6;
const int RightSensor = A7;

// TODO: change this to get more information
const int white_threshold = 100; // If it is less than this threshold it is white
const int threshold = 450; // This is the threshold hold for digital
const int black_threshold = 800; // if it is more than this threshold it is black

// IMPORTANT TODO: include min/max speed
// Define states
typedef enum {
    STATE_INITIAL,
    STATE_HUG_LEFT,
    STATE_HUG_RIGHT,
    STATE_LESS_CURVE_HUG_LEFT,
    STATE_LESS_CURVE_HUG_RIGHT
} State;

// Define state machine structure
typedef struct {
  State currentState;
  int KP;
  int KD;
  int stateCount;
  int MIN_SPEED;
  int SET_SPEED;
  int MAX_SPEED;
  int leftError;
  int middleError;
  int rightError;
  int defaultError;
} StateMachine;

// State machine is global variable
StateMachine sm;
unsigned long initialTime = millis();

void setup()
{
  // put your setup code here, to run once:
  pinMode(PWMA , OUTPUT);
  pinMode(AIN1 , OUTPUT);
  pinMode(AIN2 , OUTPUT);
  pinMode(BIN1 , OUTPUT);
  pinMode(BIN2 , OUTPUT);
  pinMode(PWMB , OUTPUT);
  pinMode(STDBY , OUTPUT);
  // TODO: I forgot to add in the pin mode code for INPUT, but it ran so test this later
  digitalWrite(STDBY , HIGH);

  // TODO: remove print statements at the end
  Serial.begin(9600); // open the serial port at 9600 bps:

  // Initialize state machine
  sm = {STATE_HUG_RIGHT, 0, 0, 0};
  stateTransition(&sm);
}
void loop()
{
  // put your main code here, to run repeatedly:

  // sensorTest();

  // TODO: optimize this reading code
  int leftValue = analogRead(LeftSensor);
  int middleValue = analogRead(MiddleSensor);
  int rightValue = analogRead(RightSensor);

  // different methods for line following
  // simpleLineFollow(leftValue, middleValue, rightValue);
  PID(leftValue, middleValue, rightValue);

  // Transition the state in the statemachine
  stateTransition(&sm);
}

/**
 * Writes either a digital or analog value to the motors, a negative speed results in 
 *
 * This function takes four integers as arguments and returns nothing.
 *
 * @param spd The speed for the analog write, negative spd results in digital write
 * @param pin_IN1 The first pin for the digital write.
 * @param pin_IN2 The second pin for the digital write
 * @param pin_PWM The pin for analog write
 * @return nothing!!!!
 */

void motorWrite(int spd, int pin_IN1 , int pin_IN2 , int pin_PWM)
{
  //TODO: figure out if this digital speed is the same as max
if (spd < 0)
{
digitalWrite(pin_IN1 , HIGH); // go one way
digitalWrite(pin_IN2 , LOW);
}
else
{
digitalWrite(pin_IN1 , LOW); // go the other way
digitalWrite(pin_IN2 , HIGH);
}
// TODO: figure out speed range with analog write
analogWrite(pin_PWM , abs(spd));
}

/**
 * This is simply motorWrite called twice with the known pins taken care of
 *
 * This function takes two integers as arguments and returns nothing.
 *
 * @param speedL The speed for the left motor
 * @param pin_IN1 The speed for the right motor
 * @return nothing!!!!
 */
void drive(int speedL, int speedR) {
    motorWrite(speedR, AIN1, AIN2, PWMA);
    motorWrite(speedL, BIN1, BIN2, PWMB);
}

void sensorTest() {
  // TODO: change this to parameter of a list of sensors, this will make it more verssatile and can choose to test specific sensors
  int leftValue = analogRead(LeftSensor);
  int middleValue = analogRead(MiddleSensor);
  int rightValue = analogRead(RightSensor);

  Serial.print("the left motor is ");
  Serial.print(leftValue, DEC);  // print as an ASCII-encoded decimal

  Serial.print(" the middle motor is ");
  Serial.print(middleValue, DEC);  // print as an ASCII-encoded decimal

  Serial.print(" the right motor is ");
  Serial.print(rightValue, DEC);  // print as an ASCII-encoded decimal
  
  Serial.println(); // make a new line for readability
  
  delay(2000);            // delay 200 milliseconds
}

void simpleLineFollow(int left, int middle, int right) {
  // TODO: move these variables somewhere else, they dont need to be redeclared everytime
  int a = 1;
  int b = 2;
  int c = 100;
 
  // TODO: we should come up with some way to train the unknown variables
  // TODO: these variables could be different depending on stuff like sharp turns and stuff. We should make state based machine for these variables for different sections
  // This is the value function
  int rightMotor = c + a*middle + b*left;
  int leftMotor = c + a*middle + b*right;

  drive(leftMotor, rightMotor);
}

// TODO: move this later
int lastError = 0;

void PID(int left, int middle, int right) {
  
  int KP = sm.KP;
  int KD = sm.KD;

  

  // This will constrain the readings
  left = constrain(left, white_threshold, black_threshold);
  middle = constrain(middle, white_threshold, black_threshold);
  right = constrain(right, white_threshold, black_threshold);

  // TODO: change this if to be more efficient
  int error;
  
  if (left == white_threshold && right == white_threshold) {
    error = sm.defaultError;
  } else {
    // this sets the sign of the middle in value function
    // NOTE: this may be backwards sign notation
    int middle_sign = -1;
    if (left > right) {
      middle_sign = -1;
    } else {
      middle_sign = 1;
    }
    // TODO: implementing code to hug the right, should be in state machine
    // straight state error = 2 * left + middle_sign * middle - 2 * right;
    // favor right state
    error = sm.leftError * left + sm.middleError * middle_sign * middle - sm.rightError * right; // GHOST VARIABLES
  }

  int adjust = error*KP - KD*(error - lastError);

  // Record the current error for the next iteration
  lastError = error;
  Serial.print("the adjust is ");
  Serial.print(adjust, DEC);
  // Adjust motors, one negatively and one positivelya
  drive(constrain(sm.SET_SPEED - adjust, sm.MIN_SPEED, sm.MAX_SPEED), constrain(sm.SET_SPEED + adjust, sm.MIN_SPEED, sm.MAX_SPEED));
}

// This is state machine code

void stateTransition(StateMachine *sm) {
    switch (sm->currentState) {
        // TODO: instead of setting values set state
        // 25 sec switch to straight
        unsigned long interval = 25000;
        unsigned long currentTime = millis();
        if ((currentTime - initialTime) > interval) {
          sm->currentState = STATE_INITIAL;
          return;
        }
        case STATE_INITIAL:
            // Initial state logic
            sm->KP = 85;
            sm->KD = 5;
            sm->leftError = 2;
            sm->middleError = 1;
            sm->rightError = 2;
            sm->defaultError = 0;
            // Speed information in example on https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
            // 0-255 for write value, 0 - 1023 for read value
            sm->MIN_SPEED = 0; // IMPORTANT NOTE: this helps control how fast can turn, the lower the more the turn
            sm->SET_SPEED = 255;
            sm->MAX_SPEED = 255;
            sm->stateCount ++;
            break;
        case STATE_HUG_LEFT:
            // State to hug the left for sharp left turns
            sm->KP = 85;
            sm->KD = 5;
            sm->stateCount ++;
            // IMPORTANT TODO: implement transition of states
            // sm->currentState = STATE_TWO;  // Transition to next state
            break;
        case STATE_HUG_RIGHT:
            // State to hug the right for sharp right turns
            sm->KP = 85;
            sm->KD = 5;
            sm->leftError = 0;
            sm->middleError = 1;
            sm->rightError = 1;
            sm->defaultError = -200;
            // Speed information in example on https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
            // 0-255 for write value, 0 - 1023 for read value
            sm->MIN_SPEED = 0; // IMPORTANT NOTE: this helps control how fast can turn, the lower the more the turn
            sm->SET_SPEED = 255;
            sm->MAX_SPEED = 255;
            sm->stateCount ++;
            break;
        case STATE_LESS_CURVE_HUG_LEFT:
            // State to hug the left for minor left turns
            sm->KP = 85;
            sm->KD = 5;
            sm->stateCount ++;
       
            break;
        case STATE_LESS_CURVE_HUG_RIGHT:
            // State to hug the right for minor right turns
            sm->KP = 85;
            sm->KD = 5;
            sm->stateCount ++;
            
            break;
        default:
            printf("Invalid State\n");
            break;
    }
}
