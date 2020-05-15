#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "WiFi-Sys"
#define STAPSK  "gdfx6327"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//Variables for the loop
char InData[35];

//Temp Variables
float Hum = 0;
float Temp2 = 0;
//RGB Variables
char RedNumId[5];
char GreenNumId[5];
char BlueNumId[5];
char ModeNumId[3];
//FanCooler var
char StaNumId[2];
char RotNumId[2];
char SpdNumId[4];
//Ultrasonic
double cm, inches;
//Door
boolean DoorStatus = false;

int SwitchScrren = 0;
// Id Var
char ScrNumId[3];
int ScrNumIdInt = 0;

uint8_t WdtCounter = 0;
uint8_t AllowSerial = 0;


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()
{
  Initial();
}

void loop() {
  // put your main code here, to run repeatedly:
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
    server.handleClient();
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
  WdtCounter++;
  if (WdtCounter >= 2)
  {
    /*ESP.wdtDisable();
    ESP.wdtEnable(50);
    ESP.wdtFeed();*/
    WdtCounter = 0;
  }
  switch(AllowSerial)
  {
    case 1:
      Serial.write("S");
      Serial.write(":");
      Serial.write(ScrNumId);
      Serial.write(",");
      AllowSerial = 0;
    break;
    case 3:
      Serial.write("R");
      Serial.write(":");
      Serial.write(RedNumId);
      Serial.write(",");
      Serial.write("G");
      Serial.write(":");
      Serial.write(GreenNumId);
      Serial.write(",");
      Serial.write("B");
      Serial.write(":");
      Serial.write(BlueNumId);
      Serial.write(",");
      Serial.write("M");
      Serial.write(":");
      Serial.write(ModeNumId);
      Serial.write(",");
      AllowSerial = 0;
    break;
    case 4:
      Serial.write("J");
      Serial.write(":");
      Serial.write(StaNumId);
      Serial.write(",");
      Serial.write("D");
      Serial.write(":");
      Serial.write(RotNumId);
      Serial.write(",");
      Serial.write("F");
      Serial.write(":");
      Serial.write(SpdNumId);
      Serial.write(",");
      AllowSerial = 0;
    break;
    default:
    break;
  }
}

//Functions of the program
void DecodeSerialData()
{
  for (int s = 0; s < 30; s++)
  {
    if (InData[s] == 'S' || InData[s] == 'T' ||InData[s] == 'H' || InData[s] == 'R' || InData[s] == 'G' || InData[s] == 'B' || InData[s] == 'M' || InData[s] == 'F'|| InData[s] == 'D' || InData[s] == 'J' || InData[s] == 'C' || InData[s] == 'I' || InData[s] == 'Z')
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
        Temp2 = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'H')
      {
        Hum = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'C')
      {
        cm = (InData2.toFloat());
      }
      else if (InData[SetPoint] == 'I')
      {
        inches = (InData2.toFloat());
      }
      else if (InData[SetPoint] == 'Z')
      {
        if (InData2 == "0")
        {
         DoorStatus = false; 
         }
        else if (InData2 == "1")
        {
          DoorStatus = true; 
        }
      }
    }
  }
}

void Initial()
{
  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  IPAddress ip(192,168,43,90);
  IPAddress gateway(192,168,43,1);
  IPAddress subnet(255,255,255,0);
  IPAddress dns(8,8,8,8);
  WiFi.disconnect();
  WiFi.hostname("ESP_AP");
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  long TstartWiFi = millis();
  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    //Serial.print(".");
    if (millis() - TstartWiFi > 500)
    {
      Serial.print(".");
      TstartWiFi = millis();
    }
    if (Serial.available() > 0 )
      {
        char t = Serial.read();
      }
    /*ESP.wdtDisable();
      ESP.wdtEnable(100);
      ESP.wdtFeed();*/
  }
  Serial.println("Ok");
  ESP.wdtFeed();
  /*Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());*/

  server.on("/", Local);
  server.on("/Temp",Temp);
  server.on("/RGB", RGB);
  server.on("/FanCooler", FanCooler);
  server.on("/Door", Door);
  server.on("/Ultrasonic", Ultrasonic);
  server.on("/ScreenNumber", ScreenNumber);
 
  server.onNotFound(handleNotFound);
  server.begin();
  //Serial.println("HTTP server started");
  for (int s = 0; s < 35; s++)
   {
      InData[s] = 0;
   }
  //delay(250);
}

void Temp()
{
  //ESP.wdtFeed();
  //Serial.end();
  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());
  String msg = "";

    const size_t bufferSize = JSON_OBJECT_SIZE(100);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject &root = jsonBuffer.createObject();
    /*String DataStrTemp = String(Temp2);
    char DataCharTemp[10];
    DataStrTemp.toCharArray(DataCharTemp,10);
    
    String DataStrHum = String(Hum);
    char DataCharHum[10];
    DataStrHum.toCharArray(10);*/
    
    root["Temp"] = double(Temp2);
    root["Hum"] = double(Hum);

    char JSONmessageBuffer[100];
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      JSONmessageBuffer[j] = 0;
    }
    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Serial.println(JSONmessageBuffer);
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      msg += JSONmessageBuffer[j];
    }
  
  server.send(200,"text/plain",msg);
  //Serial.println(msg);
  //Serial.begin(115200);
}

void ScreenNumber()
{
  //ESP.wdtFeed();
  //Serial.end();
  /*
  if (server.hasArg("plain") == false)
  {
    server.send(400,"text/plain","No Data");
    return;
  }*/
  StaticJsonBuffer<500> jsonBuffer;
  String msg = server.arg("plain");
  //Serial.println(msg);
  
  JsonObject& root = jsonBuffer.parseObject(msg);

  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());

  if (!root.success()) {
      //Serial.println("parseObject() failed");
      server.send(400,"text/plain","parseObject() failed");
      return;
    }

  const char* ScrNum = root["ScreenNumber"];
  //Serial.println(ScrNum);
  AllowSerial = 1;
  if (ScrNum)
    {
      //Serial.print("Screen Number :");
      strcpy(ScrNumId, ScrNum);
      strcat(ScrNumId, "\0");
      //Serial.println(ScrNumId);
    }
  server.send(200,"text/plain",ScrNum);
  //Serial.begin(115200);
}

void RGB()
{
  StaticJsonBuffer<500> jsonBuffer;
  String msg = server.arg("plain");
  //Serial.println(msg);
  
  JsonObject& root = jsonBuffer.parseObject(msg);

  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());

  if (!root.success()) {
      //Serial.println("parseObject() failed");
      server.send(400,"text/plain","parseObject() failed");
      return;
    }

  const char* RedNum = root["Red"];
  const char* GreenNum = root["Green"];
  const char* BlueNum = root["Blue"];
  const char* ModeNum = root["Mode"];
  //Serial.println(ScrNum);
  AllowSerial = 3;
  if (RedNum)
    {
      strcpy(RedNumId, RedNum);
      strcat(RedNumId, "\0");
      strcpy(GreenNumId, GreenNum);
      strcat(GreenNumId, "\0");
      strcpy(BlueNumId, BlueNum);
      strcat(BlueNumId, "\0");
      strcpy(ModeNumId, ModeNum);
      strcat(ModeNumId, "\0");
      
    }
  server.send(200,"text/plain","OK");
}

void FanCooler()
{
  StaticJsonBuffer<500> jsonBuffer;
  String msg = server.arg("plain");
  //Serial.println(msg);
  
  JsonObject& root = jsonBuffer.parseObject(msg);

  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());

  if (!root.success()) {
      //Serial.println("parseObject() failed");
      server.send(400,"text/plain","parseObject() failed");
      return;
    }

  const char* StaNum = root["Sta"];
  const char* RotNum = root["Rot"];
  const char* SpdNum = root["Spd"];
  //Serial.println(ScrNum);
  AllowSerial = 4;
  if (StaNum)
    {
      strcpy(StaNumId, StaNum);
      strcat(StaNumId, "\0");
      strcpy(RotNumId, RotNum);
      strcat(RotNumId, "\0");
      strcpy(SpdNumId, SpdNum);
      strcat(SpdNumId, "\0");
    }
  server.send(200,"text/plain","OK");
}

void Ultrasonic()
{
  //ESP.wdtFeed();
  //Serial.end();
  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());
  String msg = "";

    const size_t bufferSize = JSON_OBJECT_SIZE(100);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject &root = jsonBuffer.createObject();
    /*String DataStrTemp = String(Temp2);
    char DataCharTemp[10];
    DataStrTemp.toCharArray(DataCharTemp,10);
    
    String DataStrHum = String(Hum);
    char DataCharHum[10];
    DataStrHum.toCharArray(10);*/
    
    root["CM"] = cm;
    root["INCH"] = inches;

    char JSONmessageBuffer[100];
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      JSONmessageBuffer[j] = 0;
    }
    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Serial.println(JSONmessageBuffer);
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      msg += JSONmessageBuffer[j];
    }
  
  server.send(200,"text/plain",msg);
  //Serial.println(msg);
  //Serial.begin(115200);
}

void Door()
{
  //ESP.wdtFeed();
  //Serial.end();
  //Serial.print("HTTP Method: ");
  //Serial.println(server.method());
  String msg = "";

    const size_t bufferSize = JSON_OBJECT_SIZE(100);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject &root = jsonBuffer.createObject();
    /*String DataStrTemp = String(Temp2);
    char DataCharTemp[10];
    DataStrTemp.toCharArray(DataCharTemp,10);
    
    String DataStrHum = String(Hum);
    char DataCharHum[10];
    DataStrHum.toCharArray(10);*/
    int ABC = 1;
    int ABCD = 0;
    if (DoorStatus)
    {
      root["DoorStatus"] = double(ABC);
    }
    else if (!DoorStatus)
    {
      root["DoorStatus"] = double(ABCD);
    }

    char JSONmessageBuffer[100];
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      JSONmessageBuffer[j] = 0;
    }
    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Serial.println(JSONmessageBuffer);
    for (int j = 0; j < sizeof(JSONmessageBuffer); j++)
    {
      msg += JSONmessageBuffer[j];
    }
  
  server.send(200,"text/plain",msg);
  //Serial.println(msg);
  //Serial.begin(115200);
}

void Local()
{
  server.send(200,"text/plain","OK");
}
