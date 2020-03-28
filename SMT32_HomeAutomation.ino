/*
Program to test 16*2 Alaphanumeric LCD with STM32 (Blue Pill) 
For: www.circuitdigest.com

  The circuit:
 * LCD RS pin to digital pin PB11
 * LCD Enable pin to digital pin PB10
 * LCD D4 pin to digital pin PB0
 * LCD D5 pin to digital pin PB1
 * LCD D6 pin to digital pin PC13
 * LCD D7 pin to digital pin PC14
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
*/

#include <LiquidCrystal.h> // include the LCD library
//#include <TimerOne.h>

const int rs = PB11, en = PB10, d4 = PB0, d5 = PB1, d6 = PC13, d7 = PC14; //mention the pin names to with LCD is connected to 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //Initialize the LCD

//Temperature Information
#include "DHT.h"

//Ultrasonic Sensor
const int  trigPin = PA5;   // Trigger
const int  echoPin = PA4;  // Echo
long duration, cm, inches;

//Switch Screen Variable
char InData[35];
uint8_t SwitchScrren = 1;
uint8_t Screen1Counter = 0;
uint8_t TimerCounter = 0;
#define ValForChange 100
#define TiempoStep 50000

//************RGB SCREEN*******//
//Pins
#define PinRED D1
#define PinBLUE D2
#define PinGREEN D3

boolean AllowRGB = false;

//PWM LED
int REDPWM = 0;
int GREENPWM = 0;
int BLUEPWM = 0;

//***********Temp & Hum Screen ************/
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
const int DHTPin = PA7;     // Pin de Medicion
DHT dht(DHTPin, DHTTYPE);
float h = 0;
float t = 0;

void setup() {
  Initial();
  IntroSystem();
}

void loop() {
  long Tstart = millis();
  int k = 0;
  while (-Tstart + millis() < 50)
  {
    if (Serial.available() > 0 && k < 35)
    {
      InData[k] = Serial.read();
      k++;
    }
    else if (Serial.available() > 0 && k >= 35)
    {
      char t = Serial.read();
    }
  }
  if (k > 0)
  {
    DecodeSerialData();
    /*Serial.println("Msg:");
    for(int h = 0; h < k; h++)
    {
      Serial.print(InData[h]); 
    }
    Serial.println("");*/
  }
  if (SwitchScrren == 1)
  {
    if (Screen1Counter == 0)
    {
      ReadTemp();
    }
    else if (Screen1Counter == 1)
    {
      RGBData();
    }
  }
  else if (SwitchScrren == 2)
  {
    ReadTemp();
  }
  else if (SwitchScrren == 3)
  {
    RGBData();
  }
  TimerCounter++;
  if ( TimerCounter >= ValForChange)
  {
    if (SwitchScrren == 1)
    {
      Screen1Counter++;
      if (Screen1Counter >= 2)
      {
        Screen1Counter = 0;
      }
    }
    TimerCounter = 0;
    //lcd.clear(); //Clear the screen
  }
}

void Initial()
{
  Serial.begin(115200);
  //LCD beginds
  lcd.begin(16, 2);//We are using a 16*2 LCD
  Serial.println("Initialize LCD");
  //DHT22 Initialize
  Serial.println("DHT22 begins");
  dht.begin();
  //Ultrasonic 
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //RGB 
  pinMode(PinGREEN, OUTPUT);
  pinMode(PinBLUE, OUTPUT);
  pinMode(PinRED, OUTPUT);
  digitalWrite(PinRED, 0);
  digitalWrite(PinBLUE, 0);
  digitalWrite(PinGREEN, 0);
  //Timer
  //Timer1.initialize(TiempoStep);
  //Timer1.attachInterrupt(timerInterupt);
}

void DecodeSerialData()
{
  for (int s = 0; s < 30; s++)
  {
    if (InData[s] == 'S' || InData[s] == 'T' ||InData[s] == 'H')
    {
      String InData2 = "";
      int SetPoint = s;
      s = s + 2;
      while(InData[s] != ',')
      {
        InData2 += InData[s];
        s++;
      }
      if (InData[SetPoint] == 'S')
      {
        SwitchScrren = InData2.toInt();
      }
      else if (InData[SetPoint] == 'T')
      {
        t = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'H')
      {
        t = InData2.toFloat();
      }
      s--;
    }
    else if (InData[s] == 'R' || InData[s] == 'G' || InData[s] == 'B' )
    {
      String InData2 = "";
      int SetPoint = s;
      s = s + 3;
      while(InData[s] != ',')
      {
        InData2 += InData[s];
        s++;
      }
      if (InData[SetPoint] == 'R')
      {
        REDPWM = InData2.toInt();
      }
      else if (InData[SetPoint] == 'G')
      {
        GREENPWM = InData2.toInt();
      }
      else if (InData[SetPoint] == 'B')
      {
        BLUEPWM = InData2.toInt();
      }
      if (InData[SetPoint+1] == 'A')
      {
        AllowRGB = true;
      }
      else if (InData[SetPoint+1] == 'D')
      {
        AllowRGB = false;
      }
      s--;
    }
  }
}

void IntroSystem()
{
  
  lcd.setCursor(3, 0); //At first row first column 
  lcd.print("Monitoring"); //Print this
  lcd.setCursor(5, 1); //At secound row first column 
  lcd.print("System"); //Print this
  delay(2000); //wait for two secounds 
  lcd.clear(); //Clear the screen
}

void ReadTemp()
{
  // Reading temperature or humidity takes about 250 milliseconds!
   h = dht.readHumidity();
   t = dht.readTemperature();
   if (TimerCounter % 10 == 0)
   {
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed DHT sensor!");
      return;
   }
   Serial.write("H:");
   char Data1[20];
   for (int s = 0; s < 20; s++)
   {
      Data1[s] = 0;
   }
  String StrDat1 = String(h);
  StrDat1.toCharArray(Data1,20);
  for (int j = 0; j < 20; j++)
  {
    Serial.write(Data1[j]);
  }
   Serial.write(",");
   Serial.write("T:");
   char Data2[20];
   for (int s = 0; s < 20; s++)
   {
      Data2[s] = 0;
   }
  String StrDat2 = String(t);
  StrDat2.toCharArray(Data2,20);
  for (int j = 0; j < 20; j++)
  {
    Serial.write(Data2[j]);
  }
   //Serial.write("");
   Serial.println("X");
   }
   
   // Set LCD Values
  lcd.setCursor(0, 0); //At first row first column 
  lcd.print("Hum: " + String(h) + " %   "); //Print this
  lcd.setCursor(0, 1); //At secound row first column 
  lcd.print("Temp: " + String(t) + " *C  "); //Print this
}

void UltrasonicMesh()
{
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135

  char Data1[20];
  for (int s = 0; s < 20; s++)
   {
      Data1[s] = 0;
   }
  String StrDat1 = String(inches);
  StrDat1.toCharArray(Data1,20);
  for (int j = 0; j < 20; j++)
  {
    Serial.write(Data1[j]);
  }
  Serial.write("in, ");
  char Data2[20];
  for (int s = 0; s < 20; s++)
   {
      Data2[s] = 0;
   }
  String StrDat2 = String(cm);
  StrDat2.toCharArray(Data2,20);
  for (int j = 0; j < 20; j++)
  {
    Serial.write(Data2[j]);
  }
  Serial.write("cm");
  Serial.println();

  lcd.setCursor(0, 0); //At first row first column 
  lcd.print("Ultrasonic"); //Print this
  lcd.setCursor(0, 1); //At secound row first column 
  lcd.print("Dist: " + String(cm) + " cm   "); //Print this

}

void RGBData()
{
  if (AllowRGB)
  {
    analogWrite(PinRED, REDPWM);
    analogWrite(PinGREEN, GREENPWM);
    analogWrite(PinBLUE, BLUEPWM);
  }
  else
  {
    digitalWrite(PinRED, VerValor(REDPWM));
    digitalWrite(PinGREEN, VerValor(GREENPWM));
    digitalWrite(PinBLUE, VerValor(BLUEPWM));
  }
  lcd.setCursor(0, 0); //At first row first column 
  lcd.print("RGB COLOR       "); //Print this
  lcd.setCursor(0, 1); //At secound row first column 
  lcd.print("Mode: " + ModeRGB(AllowRGB) + "      "); //Print this
  //lcd.print("R:" + VerColor(REDPWM) + " G:" + String(GREENPWM) + " B:" + String(BLUEPWM)); //Print this
}

boolean VerValor(int a)
{
  if (a == 0)
  {
    return LOW;
  }
  else if (a > 0)
  {
    return HIGH;
  }  
}

String ModeRGB(boolean b )
{
  if (b == false)
  {
    return "DIG";
  }
  else if (b == true)
  {
    return "PWM";
  }
}


String VerColor(int a)
{
  if (a == 0)
  {
    return "ON";
  }
  else if (a > 0)
  {
    return "OFF";
  }  
}
