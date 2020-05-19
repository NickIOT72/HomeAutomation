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
const int  trigPin = PB14;   // Trigger
const int  echoPin = PB13;  // Echo
long duration =0;
float cm, inches;

//Switch Screen Variable
char InData[35];
uint8_t SwitchScrren = 1;
uint8_t Screen1Counter = 0;
uint8_t TimerCounter = 0;
#define ValForChange 100
#define TiempoStep 50000

//************RGB SCREEN*******//
//Pins
const int PinRED = PB6;
const int PinBLUE = PB8;
const int PinGREEN = PB7;

boolean AllowRGB = false;

//PWM LED
uint8_t REDPWM = 0;
uint8_t GREENPWM = 0;
uint8_t BLUEPWM = 0;

//***********FAN COOLER**********//
const int ENA = PA0;
const int IN1 = PA2;
const int IN2 = PA3;

#define NOROTATION false
#define ROTATION true
#define HOR true
#define ANTIHOR false
#define GainHor 2.55
boolean MotorMov = NOROTATION;
boolean Dir = HOR;
int FanSpeed  = 0;

//***********Temp & Hum Screen ************/
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
const int DHTPin = PA7;     // Pin de Medicion
DHT dht(DHTPin, DHTTYPE);
float h = 0;
float t = 0;

//*********Door Sensor*******//
const int DoorPin = PB12;
boolean DoorStatus = false;

void setup() {
  Initial();
  IntroSystem();
}

void loop()
{
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
    else if (Screen1Counter == 2)
    {
      FanCoolerMov(MotorMov, Dir);
    }
    else if (Screen1Counter == 3)
    {
      UltrasonicMesh();
    }
    else if (Screen1Counter == 4)
    {
      DoorFunct();
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
  else if (SwitchScrren == 4)
  {
    FanCoolerMov(MotorMov, Dir);
  }
  else if (SwitchScrren == 5
  )
  {
    UltrasonicMesh();
  }
  else if (Screen1Counter == 6)
  {
    DoorFunct();
  }
  TimerCounter++;
  if (TimerCounter >= ValForChange)
  {
    if (SwitchScrren == 1)
    {
      Screen1Counter++;
      if (Screen1Counter >= 5)
      {
        Screen1Counter = 0;
      }
    }
    TimerCounter = 0;
    lcd.clear(); //Clear the screen
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
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  //L294N
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(ENA, LOW);
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  //RGB 
  pinMode(PinGREEN, OUTPUT);
  pinMode(PinBLUE, OUTPUT);
  pinMode(PinRED, OUTPUT);
  analogWrite(ENA, 0);
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  //Door 
  pinMode(DoorPin, INPUT);

  analogWrite(PinRED, 0);
  analogWrite(PinBLUE, 0);
  analogWrite(PinGREEN, 0);
  delay(2000);
  analogWrite(PinRED, 255);
  analogWrite(PinGREEN, 255);
  analogWrite(PinBLUE, 255);
  delay(2000);
  analogWrite(PinRED, 123);
  analogWrite(PinGREEN, 123);
  analogWrite(PinBLUE, 123);
  delay(2000);
  analogWrite(PinRED, 0);
  analogWrite(PinBLUE, 0);
  analogWrite(PinGREEN, 0);
  //Timer
  //Timer1.initialize(TiempoStep);
  //Timer1.attachInterrupt(timerInterupt);

  //Fan Cooler Test
  FanSpeed = 100;
  FanCoolerMov(NOROTATION , HOR);
  delay(2000);
  FanCoolerMov(ROTATION , HOR);
  delay(2000);
  FanCoolerMov(NOROTATION , HOR);
  delay(2000);
  FanCoolerMov(ROTATION , ANTIHOR);
  delay(2000);
  FanCoolerMov(NOROTATION , HOR);
  delay(2000);
  FanSpeed = 0;
}

void DecodeSerialData()
{
  for (int s = 0; s < 30; s++)
  {
    if (InData[s] == 'S' || InData[s] == 'T' ||InData[s] == 'H' || InData[s] == 'R' || InData[s] == 'G' || InData[s] == 'B' || InData[s] == 'M' || InData[s] == 'F'|| InData[s] == 'D' || InData[s] == 'J' || InData[s] == 'C' || InData[s] == 'I')
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
        lcd.clear();
      }
      else if (InData[SetPoint] == 'T')
      {
        t = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'H')
      {
        h = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'R')
      {
        REDPWM =InData2.toInt();
        /*Serial.println("");
        Serial.print("Red: ");
        Serial.println(REDPWM);*/
      }
      else if (InData[SetPoint] == 'G')
      {
        GREENPWM = InData2.toInt();
        /*Serial.println("");
        Serial.print("Green: ");
        Serial.println(GREENPWM);*/
      }
      else if (InData[SetPoint] == 'B')
      {
        BLUEPWM = InData2.toInt();
        /*Serial.println("");
        Serial.print("Blue: ");
        Serial.println(BLUEPWM);*/
      }
      else if (InData[SetPoint] == 'M')
      {
        if (InData2 == "A" )
        {
          AllowRGB = true;
        }
        else if (InData2 == "D")
        {
          AllowRGB = false;
        }
      }
      else if (InData[SetPoint] == 'F' )
      {
        FanSpeed = (InData2.toInt());
      }
      else if (InData[SetPoint] == 'D')
      {
        if (InData2 == "0")
        {
          Dir = false; 
        }
        else if (InData2 == "1")
        {
          Dir = true; 
        }
      }
      else if (InData[SetPoint] == 'J')
      {
        if (InData2 == "0")
        {
          MotorMov = false; 
        }
        else if (InData2 == "1")
        {
          MotorMov = true; 
        }
      }
      else if (InData[SetPoint] == 'C')
      {
        cm = (InData2.toFloat());
      }
      else if (InData[SetPoint] == 'I')
      {
        inches = (InData2.toFloat());
      }
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
  if (TimerCounter % 10 == 0)
  {
    lcd.clear();
    h = dht.readHumidity();
    t = dht.readTemperature();
    Serial.write("H:");
    char Data1[20];
    for (int s = 0; s < 20; s++)
    {
      Data1[s] = 0;
    }
    String StrDat1 = String(h);
    StrDat1.toCharArray(Data1, 20);
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
    StrDat2.toCharArray(Data2, 20);
    for (int j = 0; j < 20; j++)
    {
      Serial.write(Data2[j]);
    }
    Serial.write(",");
    //Serial.write("");
  }

  // Set LCD Values
  lcd.setCursor(0, 0);                       //At first row first column
  lcd.print("Hum: " + String(h) + " %   ");  //Print this
  lcd.setCursor(0, 1);                       //At secound row first column
  lcd.print("Temp: " + String(t) + " *C  "); //Print this
}

void UltrasonicMesh()
{
  
  if (TimerCounter % 10 == 0)
  {
    lcd.clear();
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
    Serial.write("C:");
    char Data1[20];
    for (int s = 0; s < 20; s++)
    {
      Data1[s] = 0;
    }
    String StrDat1 = String(cm);
    StrDat1.toCharArray(Data1, 20);
    for (int j = 0; j < 20; j++)
    {
      Serial.write(Data1[j]);
    }
    Serial.write(",");
    Serial.write("I:");
    char Data2[20];
    for (int s = 0; s < 20; s++)
    {
      Data2[s] = 0;
    }
    String StrDat2 = String(inches);
    StrDat2.toCharArray(Data2, 20);
    for (int j = 0; j < 20; j++)
    {
      Serial.write(Data2[j]);
    }
    Serial.write(",");
    //Serial.println();
    
  }
  
    lcd.setCursor(0, 0); //At first row first column 
    lcd.print("Dist: " + String(int(inches)) + " in   "); //Print this
    lcd.setCursor(0, 1); //At secound row first column 
    lcd.print("      " + String(int(cm)) + " cm   "); //Print this
}

void DoorFunct()
{
  DoorStatus = digitalRead(DoorPin); // Lee el valor del sensor

  if (TimerCounter % 10 == 0)
  {
    Serial.write("Z:");
    char Data1[2];
    for (int s = 0; s < 20; s++)
    {
      Data1[s] = 0;
    }
    String StrDat2 = "";
    if (DoorStatus == true)
    {
      StrDat2 = "1";
    }
    else if (DoorStatus == false)
    {
      StrDat2 = "0";
    }
    StrDat2.toCharArray(Data1, 2);
    for (int j = 0; j < 2; j++)
    {
      Serial.write(Data1[j]);
    }
    Serial.write(",");
  }

  // Set LCD Values
  lcd.setCursor(0, 0);                       //At first row first column
  lcd.print("  DOOR STATUS  ");  //Print this
  lcd.setCursor(0, 1);                       //At secound row first column
  if (DoorStatus == true)
  {
    lcd.print("DOOR CLOSE  "); //Print this
  }
  else if (DoorStatus == false)
  {
    lcd.print("DOOR OPEN  "); //Print this
  }
}

void RGBData()
{
  analogWrite(PinRED, REDPWM);
  analogWrite(PinGREEN, GREENPWM);
  analogWrite(PinBLUE, BLUEPWM);
  if (TimerCounter % 4 == 0)
  {
    lcd.clear();
  }
  lcd.setCursor(0, 0); //At first row first column 
  lcd.print("RGB COLOR"); //Print this
  lcd.setCursor(0, 1); //At secound row first column 
  lcd.print("HEX:#"); //Print this
  lcd.setCursor(5, 1); //At secound row first column
  lcd.print(REDPWM,HEX); //Print this
  lcd.setCursor(7, 1); //At secound row first column
  lcd.print(GREENPWM,HEX); //Print this
  lcd.setCursor(9, 1); //At secound row first column
  lcd.print(BLUEPWM,HEX); //Print this
}

int VerValor(int a)
{
  if (a == 0)
  {
    return 0;
  }
  else if (a > 0)
  {
    return 255;
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
    return "ON ";
  }
  else if (a > 0)
  {
    return "OFF";
  }  
}

void FanCoolerMov(boolean a , boolean b)
{
  if (a == false)
  {
    digitalWrite(ENA, LOW);
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
  }
  else if (a == true)
  {
    digitalWrite(ENA, HIGH);
    if (b == false)
    {
      analogWrite(IN2, 0);
      analogWrite(IN1, 255/100*FanSpeed);
    }
    else if (b == true)
    {
      analogWrite(IN1, 0);
      analogWrite(IN2, 255/100*FanSpeed);
    }
  }

  lcd.setCursor(0, 0); //At first row first column 
  lcd.print("COOLER STA:" + ModeFAN(a)); //Print this
  lcd.setCursor(0, 1); //At secound row first column 
  lcd.print("ROT:" + ModeFANHOR(b) + " SPD:   "); //Print this
  lcd.setCursor(12, 1); //At secound row first column 
  lcd.print(String(FanSpeed) + "%"); //Print this
}

String ModeFAN(boolean c)
{
 if (c == ROTATION)
 {
  return "ON ";
 }
 else if (c == NOROTATION)
 {
  return "OFF";
 }
}

String ModeFANHOR(boolean c)
{
 if (c == false)
 {
  return "HOR";
 }
 else if (c == true)
 {
  return "AHR";
 }
}
