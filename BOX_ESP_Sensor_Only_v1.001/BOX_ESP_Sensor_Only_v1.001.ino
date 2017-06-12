/*
 * 結果：
 * うまく動いた
 * 
 * 作業：
 * 単体のセンシングに特化
 * 
 */

#include <ESP8266WiFi.h>

int client_wait = 200;

long lastMsg = 0;
char msg[256];
int value = 0;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * センサー初期設定
 */

// #include "Arduino.h"

class Ultrasonic
{
    public:
    Ultrasonic(int pin);
    void DistanceMeasure(void);
    long microsecondsToCentimeters(void);
    long microsecondsToInches(void);
    private:
    int _pin;//pin number of Arduino that is connected with SIG pin of Ultrasonic Ranger.
    long duration;// the Pulse time received;
};
Ultrasonic::Ultrasonic(int pin)
{
    _pin = pin;
}
/*Begin the detection and get the pulse back signal*/
void Ultrasonic::DistanceMeasure(void)
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(_pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(_pin,LOW);
    pinMode(_pin,INPUT);
    duration = pulseIn(_pin,HIGH);
}
/*The measured distance from the range 0 to 400 Centimeters*/
long Ultrasonic::microsecondsToCentimeters(void)
{
    return duration/29/2;
}
/*The measured distance from the range 0 to 157 Inches*/
long Ultrasonic::microsecondsToInches(void)
{
    return duration/74/2;
}

///////////////////////////////////////////////////////////////////////////////////////

// センサー取得用初期設定

// Ultrasonic
Ultrasonic ultrasonic(14);

// ADXL345
#include <Wire.h>
#include <ADXL345.h>

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

// ITG3200
#include <Wire.h>
#include "ITG3200.h"
ITG3200 gyro;

//////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();
  
  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }

  WiFi.mode(WIFI_OFF);

  setup_sensor();
}

long sound_sum;
long RangeInCentimeters;
int x,y,z;
double xyz[3];
double ax,ay,az;
float gx,gy,gz;
float temp_c;

void loop() {
      
      long now = millis();
      if (now - lastMsg > client_wait) {
        lastMsg = now;
        ++value;

        get_sensor();
        
        if(value == 1){
          Serial.print("{ \"message\": \"FirstAccess!!!!!!\" ");
          Serial.println(" }");
        } else {
          Serial.print("{");
          Serial.print("\"s\":");
          Serial.print(sound_sum);
          Serial.print(",\"r\":");
          Serial.print(RangeInCentimeters);
          
          Serial.print(",\"t\":");
          Serial.print(temp_c);
          
          Serial.print(",\"ax\":");
          Serial.print(ax);
          Serial.print(",\"ay\":");
          Serial.print(ay);
          Serial.print(",\"az\":");
          Serial.print(az);

          Serial.print(",\"gx\":");
          Serial.print(gx);
          Serial.print(",\"gy\":");
          Serial.print(gy);
          Serial.print(",\"gz\":");
          Serial.print(gz);
          
          Serial.print(",\"c\":");
          Serial.print(value);
          Serial.println("}");
        }
      }
}

void setup_sensor() {

  // 加速度  /////////////////////////////////////////////////////////////
  adxl.powerOn();
  
  //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75); //62.5mg per increment
  adxl.setInactivityThreshold(75); //62.5mg per increment
  adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?
  
  //look of activity movement on this axes - 1 == on; 0 == off
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
  
  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);
  
  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);
  
  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50); //62.5mg per increment
  adxl.setTapDuration(15); //625us per increment
  adxl.setDoubleTapLatency(80); //1.25ms per increment
  adxl.setDoubleTapWindow(200); //1.25ms per increment
  
  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment
  
  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );
  
  //register interrupt actions - 1 == on; 0 == off
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  1);
  adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 1);
  
  // ジャイロ /////////////////////////////////////////////////////////////
  gyro.init();
  gyro.zeroCalibrate(200,10);//sample 200 times to calibrate and it will take 200*10ms
  
}

void get_sensor() {

  // 音圧センサー
  sound_sum = 0;
  for(int i=0; i<32; i++)
  {
      sound_sum += analogRead(A0);
  }
  sound_sum >>= 5;
  
  // 距離センサー
  RangeInCentimeters;
  ultrasonic.DistanceMeasure();// get the current signal time;
  RangeInCentimeters = ultrasonic.microsecondsToCentimeters();//convert the time to centimeters
  // 
  // adxl
  adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  adxl.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];
  
  // gyro

  temp_c = gyro.getTemperature();
  
  /*
  Serial.print("Temperature = ");
  Serial.print(gyro.getTemperature());
  Serial.println(" C");
  */

  /*
  int16_t x,y,z;
  gyro.getXYZ(&x,&y,&z);
  Serial.print("values of X , Y , Z: ");
  Serial.print(x);
  Serial.print(" , ");
  Serial.print(y);
  Serial.print(" , ");
  Serial.println(z);
  */
  
  gyro.getAngularVelocity(&gx,&gy,&gz);
  
  /*
  Serial.print("Angular Velocity of X , Y , Z: ");
  Serial.print(gx);
  Serial.print(" , ");
  Serial.print(gy);
  Serial.print(" , ");
  Serial.print(gz);
  Serial.println(" degrees per second");
  Serial.println("*************");
  */

  /*
  Serial.print("RangeInCentimeters ");
  Serial.println(RangeInCentimeters);

  Serial.print("temp_c ");
  Serial.println(temp_c);
  */

  /*
  Serial.print(" ax ");
  Serial.print(ax);
  Serial.print(" ay ");
  Serial.print(ay);
  Serial.print(" az ");
  Serial.println(az);
  */

  /*
  Serial.print(" gx ");
  Serial.print(gx);
  Serial.print(" gy ");
  Serial.print(gy);
  Serial.print(" gz ");
  Serial.println(gz);
  */

  /*
  char gz_int2[7];
  dtostrf(gx,-8,2,gz_int2);
  Serial.print(" gz_int2 ");
  Serial.println(gz_int2);
  Serial.print(" gz_int2.strlen ");
  Serial.println(strlen(gz_int2));
  */
  
}
