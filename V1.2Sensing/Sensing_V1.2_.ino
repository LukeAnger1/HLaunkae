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
digitalWrite(STDBY , HIGH);

// TODO: remove print statements at the end
Serial.begin(9600); // open the serial port at 9600 bps:
}
void loop()
{
  // put your main code here, to run repeatedly:
  // drive(100,100); //drive forwards
  sensorTest();
}
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

// your drive() function goes here!
void drive(int speedL, int speedR) {
    motorWrite(speedL, AIN1, AIN2, PWMA);
    motorWrite(speedR, BIN1, BIN2, PWMB);
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
  
  delay(2000);            // delay 200 milliseconds
}
