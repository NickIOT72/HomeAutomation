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
double Hum = 0;
double Temp2 = 0;
char ScrNumId[10];
int ScrNumIdInt = 0;
uint8_t WdtCounter = 0;
boolean AllowScreenSerial = false;

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
  if (AllowScreenSerial)
  {
    Serial.write("S");
    Serial.write(":");
    Serial.write(ScrNumId);
    Serial.write(",");
    AllowScreenSerial = false;
  }
}

//Functions of the program
void DecodeSerialData()
{
  for (int s = 0; s < 30; s++)
  {
    if (InData[s] == 'H' || InData[s] == 'T')
    {
      String InData2 = "";
      int SetPoint = s;
      s = s + 2;
      while(InData[s] != ',')
      {
        InData2 += InData[s];
        s++;
      }
      if (InData[SetPoint] == 'H')
      {
        Hum = InData2.toFloat();
      }
      else if (InData[SetPoint] == 'T')
      {
        Temp2 = InData2.toFloat();
      }
      s--;
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
    
    root["Temp"] = Temp2;
    root["Hum"] = Hum;

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
  AllowScreenSerial = true;
  if (ScrNum)
    {
      //Serial.print("Screen Number :");
      strcpy(ScrNumId, ScrNum);
      strcat(ScrNumId, "\0");
      //Serial.println(ScrNumId);
    }
  server.send(200,"text/plain","OK");
  //Serial.begin(115200);
}

void Local()
{
  server.send(200,"text/plain","OK");
}
