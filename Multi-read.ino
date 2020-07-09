
/*
  ESP8266 read pin A0 using multiplexer and send to Blynk
*/

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClient.h>

//Global constants
char auth[] = "ePrTwTZ_LKedEAtcjTQdh6bwC_emNx2L";
char ssid[] = "ZyXEL_E3ED";
char pass[] = "PH8LUC3HLK";
long TEN_MINUTES = 600000L;
//char server[] ="91.152.216.101";
char server[] ="192.168.1.158";

WiFiClient client; 

//defaults
float temperatureLowLimit = 18.0;
float lightLowLimit = 50.0;
float moistureLowLimit = 25.0;

//pins used
int sensorPin = A0;
int enable0 = 12;
int enable1 = 14;
int enable2 = 13;
int enable3 = 15;

//Global variables
BlynkTimer timer;
int temperatureSensorValue = 0;
float moistureSensorValue = 0;
float lightSensorValue = 0;
WidgetTerminal terminal(V5);
WidgetLED ledStatus(V3);

//converted
int temperatureReal = 0;
int moistureReal = 0;
int lightReal = 0;

void measuringTimer()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  timer.setTimeout(300, ReadTemperatureSensor); 
  //timer.setTimeout(300, Sending_To_phpmyadmindatabase );
}

void notificationTimer(){
  timer.setTimeout(1000, sendNotification); 
}

void sendNotification()
{
    if(temperatureReal<temperatureLowLimit || ((1024 - moistureSensorValue)*100/1024)< moistureLowLimit || (lightSensorValue/1024*100)<lightLowLimit){
    terminal.println(temperatureReal);
    terminal.println(((1024 - moistureSensorValue)*100/1024));
    terminal.println((lightSensorValue/1024*100));
    Blynk.setProperty(V3, "color", "#D3435C"); //red
    Blynk.notify("Check the status of your plant at device {DEVICE_NAME}");
    }
   else{
    Blynk.setProperty(V3, "color", "#23C48E"); //green
    }
}
void setup() {
  Serial.begin(115200);
  StartWiFi();
  Blynk.begin(auth, ssid, pass);
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);
  pinMode(enable3, OUTPUT);
  pinMode(enable0, OUTPUT);
  pinMode(sensorPin, INPUT);
  //get measurements every 1s
  timer.setInterval(1000L, measuringTimer);
  //send check plant notification every 10 mins
  timer.setInterval(TEN_MINUTES, notificationTimer);
  //save to mysql every 10 mins
  timer.setInterval(TEN_MINUTES, Sending_To_phpmyadmindatabase );
  // Clear the terminal content
  terminal.clear();

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("Connected to network: " + String(ssid));
  terminal.println(F("-------------"));
  terminal.println(F("Status OK.."));
  terminal.println(F("Available commands: /requestData, /restart, /setTemperature, /setLight, /setMoisture"));
  ledStatus.on();
  Blynk.setProperty(V3, "color", "#23C48E");
  terminal.flush();
}

void loop() {
  Blynk.run();
  timer.run();

}
void StartWiFi(){
 Serial.println('\n');
  
  WiFi.begin(ssid, pass);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer  

}

BLYNK_WRITE(V5)
{

  String testString = param.asStr();
  if (String("/restart") == param.asStr()) {
    terminal.println("RESTARTING {DEVICE_NAME}");
    terminal.flush();
    softRestart();
  }
  
  if (String("/requestData") == param.asStr()) {
    terminal.println("Raw data requested..");
    
    terminal.print(("Raw temperature value is "));
    terminal.println((String(temperatureSensorValue)));

    terminal.print(("Raw light value is "));
    terminal.println((String(lightSensorValue)));

    terminal.print(("Raw moisture value is "));
    terminal.println((String(moistureSensorValue)));
    
    terminal.flush();
  }  
 if (testString.startsWith("/setTemperature"))
 {
    terminal.println("Set new temperature limits: ");
    terminal.println(("New temperature low limit (C) is: "));
    testString.remove(0,16);
    temperatureLowLimit = testString.toFloat();
    terminal.println(testString);
    terminal.flush();
 }
 if (testString.startsWith("/setLight"))
 {
    terminal.println("Set new light limits: ");
    terminal.println(("New light low limit is (%): "));
    testString.remove(0,10);
    lightLowLimit = testString.toFloat();
    terminal.println(testString);
    terminal.flush();
 }
if (testString.startsWith("/setMoisture"))
 {
    terminal.println("Set new moisture limits: ");
    terminal.println(("New moisture low limit is (%): "));
    testString.remove(0,13);
    moistureLowLimit = testString.toFloat();
    terminal.println(testString);
    terminal.flush();
 }
  
}

void ReadTemperatureSensor() {

  //highest value reached indoors:
  digitalWrite(enable0, HIGH);
  digitalWrite(enable1, HIGH);
  digitalWrite(enable2, LOW);
  digitalWrite(enable3, LOW);
  temperatureSensorValue = analogRead(sensorPin);
  Serial.print("Temperature sensor value is: ");
  Serial.println(temperatureSensorValue);
  //terminal.print(("Raw temperature value is "));
  //terminal.println((String(temperatureSensorValue)));
  temperatureReal = temperatureSensorValue*3300.0/1023.0/10.0;
  timer.setTimeout(300, ReadLightSensor); 
}

void ReadLightSensor() {
  // read the value from sensor B:
  //highest value reached indoors: 960
  digitalWrite(enable0, LOW);
  digitalWrite(enable1, LOW);
  digitalWrite(enable2, HIGH);
  digitalWrite(enable3, LOW);
  lightSensorValue = analogRead(sensorPin);
  Serial.print("Light sensor value is: ");
  Serial.println(lightSensorValue);
  //terminal.print(("Raw light value is "));
  //terminal.println((String(lightSensorValue)));
  timer.setTimeout(300, ReadMoistureSensor); 
}

void ReadMoistureSensor() {
  // read the value from sensor C:
  //highest value reached indoors:
  digitalWrite(enable0, HIGH);
  digitalWrite(enable1, LOW);
  digitalWrite(enable2, HIGH);
  digitalWrite(enable3, LOW);
  moistureSensorValue = analogRead(sensorPin);
  Serial.print("Moisture sensor value is: ");
  Serial.println(moistureSensorValue);
  Serial.println("----------------------------------------");
  //terminal.print(("Raw moisture value is "));
  //terminal.println((String(moistureSensorValue)));
  timer.setTimeout(300, writeToBlynk); 
}

void writeToBlynk(){
  
  Blynk.virtualWrite(V0, temperatureReal);
  Blynk.virtualWrite(V1, (1024 - moistureSensorValue)*100/1024);
  Blynk.virtualWrite(V2, (lightSensorValue)/1024*100);
}

void softRestart(){
  ESP.restart();  
}


void Sending_To_phpmyadmindatabase()   //CONNECTING WITH MYSQL
 {
   if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    Serial.print("GET /nodemcu.php?moisture=");
    client.print("GET /nodemcu.php?moisture=");     //YOUR URL
    Serial.println(moistureSensorValue);
    float moist = (1024 - moistureSensorValue)*100/1024;
    client.print(moist);
    client.print("&temperature=");
    Serial.println("&temperature=");
    float temp = temperatureSensorValue*3300.0/1023.0/10.0;
    client.print(temp);
    Serial.println(temperatureSensorValue);
    client.print("&light=");
    client.print((lightSensorValue)/1024*100);
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    //client.println("Host: 192.168.1.55");
    client.println("Host: " + String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3]);
    //Serial.println("Local IP is" +  String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3]);
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    }
  }

  String toString(const IPAddress& address){
  return String() + address[0] + "." + address[1] + "." + address[2] + "." + address[3];
}
