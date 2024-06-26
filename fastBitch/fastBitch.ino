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

const int LeftSensor = A5; // sensors looking from the robots perspective
const int MiddleSensor = A6;
const int RightSensor = A7;

const int white_threshold = 100; // If it is less than this threshold it is white
const int threshold = 450; // This is the threshold hold for digital
const int black_threshold = 800; // if it is more than this threshold it is black

// Define states
// NOTE: Our robot works by defining states that have optimized variables for passing obstacles on the course
// TODO: currently time and a counter to reset time. This can be better, see issue tracker in repo
typedef enum {
    STATE_STRAIGHT,
    STATE_STRAIGHT_WITH_COUNTING,
    STATE_HUG_LEFT,
    STATE_HUG_RIGHT,
    STATE_BEAR_HUG_LEFT,
    STATE_BEAR_HUG_RIGHT,
    STATE_BEAR_BEAR_HUG_LEFT,
    STATE_CIRCLE
} State;

// Define state machine structure
State currentState;
int KP;
int KD;
int stateCount;
int MIN_SPEED;
int SET_SPEED;
int MAX_SPEED;
int defaultError;
int readLeftWeight = 1;
int readMiddleWeight = 0;
int readRightWeight = -1;

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
  digitalWrite(STDBY , HIGH); // TODO: test removing this line

  Serial.begin(9600); // open the serial port at 9600 bps:

currentState = STATE_STRAIGHT; // This is the initial state for the robot
}
void loop()
{
  // put your main code here, to run repeatedly:

  int leftValue = analogRead(LeftSensor); // retreive sensor values
  int middleValue = analogRead(MiddleSensor);
  int rightValue = analogRead(RightSensor);

  // different methods for line following
  // simpleLineFollow(leftValue, middleValue, rightValue);
  PID(leftValue, middleValue, rightValue);

  // Transition the state in the statemachine
  stateTransition();
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
  
  delay(2000);            // delay 2 seconds
}

void simpleLineFollow(int left, int middle, int right) {
  // TODO: move these variables somewhere else, they dont need to be redeclared everytime
  int a = 1;
  int b = 2;
  int c = 100;
 
  // This is the value function
  // NOTE: This is good starter code but it slows down dependent on the value, so sub optimal
  int rightMotor = c + a*middle + b*left;
  int leftMotor = c + a*middle + b*right;

  drive(leftMotor, rightMotor);
}

// This is used for the derivative in PID
int lastError = 0;

// This keeps track of how many white we hit when driving
// NOTE: This will switch states, I like these conditions that  whertake the course into account to know when to switch states, it is better
bool wasPreviousWhite = false;
int whiteCount = 0;

void PID(int left, int middle, int right) {

  // This will constrain the readings
  // NOTE: These constraints allow confidence in a value being black or white
  left = constrain(left, white_threshold, black_threshold);
  middle = constrain(middle, white_threshold, black_threshold);
  right = constrain(right, white_threshold, black_threshold);

  // TODO: change this if to be more efficient
  int error;
  if (left == white_threshold && right == white_threshold) {
    error = defaultError; // NOTE: This conditional is essential for our robot, as it switches states this is the conditional to do if it cant read anything
    // EXAMPLE: hard rights can be performed by turning right when there no line to follow

    // This code counts how many white sections we have encountered while going straight, it is used to reset the clock counter to prevent cascading errors
    if (currentState == STATE_STRAIGHT_WITH_COUNTING && !wasPreviousWhite) {
      wasPreviousWhite = true;
      whiteCount++;
    }
  } else {
    wasPreviousWhite = false;

    // favor right state
    error = readLeftWeight * left + readMiddleWeight * middle + readRightWeight * right;
  }

  int adjust = error*KP - KD*(error - lastError);

  // NOTE: This techniquely isn't PID because the error isnt getting updated, but the robot runs smoother and faster without this
  // Record the current error for the next iteration
  // lastError = error;

  // Adjust motors, one negatively and one positively
  // NOTE: We have to constrain the readings to prevent issues such as driving backwards or attempting to drive beyond max speed
  drive(constrain(SET_SPEED - adjust, MIN_SPEED, MAX_SPEED), constrain(SET_SPEED + adjust, MIN_SPEED, MAX_SPEED));
}

unsigned long startTime = millis();

// This variable says if we should run the second half of the course
bool runSecondHalf = false;

// This is state machine code
void stateTransition() {
    // This changes states based off the time
    unsigned long currentTime = millis();

    // This will reset the startTime, it will also make sure we run the second half
    if (!runSecondHalf && whiteCount >= 2) {
      runSecondHalf = true;
      startTime = millis();
    }

    if (runSecondHalf) {
      // This is the second half of the course
      if (currentTime - startTime > 10000) {
        currentState = STATE_CIRCLE;
      } else if (currentTime - startTime > 2000) {
        currentState = STATE_BEAR_HUG_RIGHT;
      } else if (currentTime - startTime > 1000) {
        currentState = STATE_BEAR_BEAR_HUG_LEFT;
      }
    } else {
      // This is the start of the match code
      if (currentTime - startTime > 23000) {
        currentState = STATE_STRAIGHT_WITH_COUNTING;
      } else if (currentTime - startTime > 18000) {
        currentState = STATE_BEAR_HUG_RIGHT;
      } else if (currentTime - startTime > 10000) {
        currentState = STATE_STRAIGHT;
      } else {
        currentState = STATE_HUG_RIGHT;
      }
    }

    // NOTE: This is a spot to test states, it will override the state machine
    // currentState = STATE_CIRCLE;
  
    switch (currentState) {
        case STATE_STRAIGHT:
            // Straight state logic
            KP = 20; // This variable dictates how much to correct
            KD = 19; // This is the derivative to make turning more smooth, but right now it isnt used correctly so may need to remove
            // Speed information in example on https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
            // 0-255 for write value, 0 - 1023 for read value
            MIN_SPEED = 0; // IMPORTANT NOTE: this helps control how fast can turn, the lower the more the turn
            SET_SPEED = 255; // This is the goal speed
            MAX_SPEED = 255; // This is the max speed
            defaultError = 0; // This is what to do when there is only white, really good for sharp turns
            readLeftWeight = 1; // There is an error function that weights the sensors based off of these three values
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++; // This is to count how many state transitions have taken place, currently not used
            break;
        case STATE_STRAIGHT_WITH_COUNTING:
            // Straight with counting logic
            KP = 20;
            KD = 18;
            MIN_SPEED = 0;
            SET_SPEED = 255;
            MAX_SPEED = 255;
            defaultError = 0;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_HUG_LEFT:
            // State to hug the left for minor left turns
            KP = 20;
            KD = 17;
            MIN_SPEED = 0;
            SET_SPEED = 255;
            MAX_SPEED = 255;
            defaultError = 200;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_HUG_RIGHT:
            // State to hug the right for minor right turns
            KP = 20;
            KD = 17;
            MIN_SPEED = 0;
            SET_SPEED = 255;
            MAX_SPEED = 255;
            defaultError = -200;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_BEAR_HUG_LEFT:
            // State to hug the left for sharp left turns
            KP = 20;
            KD = 5;
            MIN_SPEED = 0;
            SET_SPEED = 255;
            MAX_SPEED = 255;
            defaultError = 200;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_BEAR_HUG_RIGHT:
            // State to hug the right for sharp right turns
            KP = 20;
            KD = 5;
            MIN_SPEED = 0;
            SET_SPEED = 255;
            MAX_SPEED = 255;
            defaultError = -200;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_BEAR_BEAR_HUG_LEFT:
            // State go slower for super sharp left turns
            KP = 20;
            KD = 5;
            MIN_SPEED = -200;
            SET_SPEED = 245;
            MAX_SPEED = 245;
            defaultError = 100;
            readLeftWeight = 1;
            readMiddleWeight = 0;
            readRightWeight = -1;
            stateCount ++;
            break;
        case STATE_CIRCLE:
            // State to go around the right side of the circle
            // NOTE: I never finished this code. The robot finished with this state once. I dont know how and I said FUCK it it worked.
            // IMPORTANT NOTE: I still got a shirt
            KP = 20;
            KD = 15;
            MIN_SPEED = -200;
            SET_SPEED = 180;
            MAX_SPEED = 180;
            defaultError = -200;
            stateCount ++;
            readLeftWeight = 0;
            readMiddleWeight = 1;
            readRightWeight = -1;
            break;
        default:
            // WTF Are you running?!?
            printf("Invalid State\n");
            break;
    }
}
