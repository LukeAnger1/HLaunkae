// I worked with Bruce Willis, that guy from Modern Family, Ryan Reynolds left shoe, Tobey Macguire, and batman to write this code

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

// IMPORTANT TODO: find the min and max speed
const int MIN_SPEED = 0;
const int SET_SPEED = 100;
const int MAX_SPEED = 100;

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
}
void loop()
{
  // put your main code here, to run repeatedly:
  //drive(0,200); // this makes the left motor be the only one to turn
  // sensorTest();

  // TODO: optimize this reading code
  int leftValue = analogRead(LeftSensor);
  int middleValue = analogRead(MiddleSensor);
  int rightValue = analogRead(RightSensor);
  
  simpleLineFollow(leftValue, middleValue, rightValue);
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
  // TODO: the code bellow is for digital write, we should remove later
  /*
  if (leftValue == middleValue && middleValue == rightValue) { // check if all black or all white
    drive(100, 100); // drive forward
  }

  if (leftValue == 0 && middleValue == 1 && rightValue == 1) { // check if all black or all white
    drive(50, 100); // slight right
  }

  if (leftValue == 0 && middleValue == 0 && rightValue == 1) { // check if all black or all white
    drive(0, 150); // hard right
  }

  if (leftValue == 1 && middleValue == 1 && rightValue == 0) { // check if all black or all white
    drive(100, 50); // slight left
  }

  if (leftValue == 1 && middleValue == 0 && rightValue == 0) { // check if all black or all white
    drive(150, 0); // hard left
  }*/
}

// TODO: move this later
int lastError = 0;

void PID(int left, int middle, int right) {
  // TODO: move these
  int KP = 1;
  int KD = 1;

  // This will constrain the readings
  left = constrainy(left, white_threshold, black_threshold);
  middle = constrainy(middle, white_threshold, black_threshold);
  right = constrainy(right, white_threshold, black_threshold);

  // TODO: change this if to be more efficient
  int error;
  if (left == white_threshold && right == white_threshold) {
    error = 0;
  } else {
    // this sets the sign of the middle in value function
    // NOTE: this may be backwards sign notation
    int middle_sign;
    if (left > right) {
      middle_sign = -1;
    } else {
      middle_sign = 1;
    }
    error = 2 * left + middle_sign * middle - 2 * right;
  }

  // NOTE: this is the begining of the old code for reference
  // Take a reading
  // unsigned int linePos = qtrSensors.readLine(sensorValues);

  // Compute the error
  // int error = SETPOINT - linePos;
  // NOTE: this is the end of the old code for reference

  // TODO: may want to include another derivative 
  // Compute the motor adjustment
  int adjust = error*KP + KD*(error - lastError);

  // Record the current error for the next iteration
  lastError = error;

  // Adjust motors, one negatively and one positivelya
  drive(constrainy(SET_SPEED - adjust, MIN_SPEED, MAX_SPEED), constrain(SET_SPEED + adjust, MIN_SPEED, MAX_SPEED));
}

int constrainy(int value, int miny, int maxy) {
  // this function makes sure the value is inbetween the given constriants
  if (value < miny) {
    return miny;
  }

  if (value > maxy) {
    return maxy;
  }

  return value;
}
