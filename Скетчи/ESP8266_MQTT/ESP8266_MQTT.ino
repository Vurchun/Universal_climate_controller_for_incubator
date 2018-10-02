#include <ArduinoJson.h>
#include <MQTT.h>
#include <PubSubClient.h>
#include <PubSubClient_JSON.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#define ssid "WiFi"   
#define pass "10072003WiFi"     
#define mqtt_server "m21.cloudmqtt.com"
#define mqtt_port 19787 
#define mqtt_user "udciowys"
#define mqtt_pass "ZXBwDFYgAU_K"
#define mqtt_client_name "IncubatorAdmin" // Client connections cant have the same connection name
#define BUFFER_SIZE 100
unsigned long previousMillis = 0;
const long interval = 1000;   
WiFiClient wclient;  //Declares a WifiClient Object using ESP8266WiFi
PubSubClient client(wclient, mqtt_server, mqtt_port);  //instanciates client object
//Function is called when, a message is recieved in the MQTT server.
void setup() {
// Setup console
Serial.begin(115200);  //set the baud rate
delay(10);
Serial.println();
}
void SendEsp()
{ 
  if(Serial.parseInt() == 1 ){
    Serial.println("Start");
    float Temp = Serial.parseFloat(); client.publish("Incubator/Temp",String(Temp));Serial.println(Temp);
    float ControlTemp = Serial.parseFloat(); client.publish("Incubator/ControlTemp",String(ControlTemp));Serial.println(ControlTemp);
    int Hum = Serial.parseInt(); client.publish("Incubator/Hum",String(Hum));Serial.println(Hum);
    int ControlHum = Serial.parseInt(); client.publish("Incubator/ControlHum",String(ControlHum));Serial.println(ControlHum);
    int Day = Serial.parseInt(); client.publish("Incubator/Day",String(Day));Serial.println(Day);
    Serial.println("Finish");
    Serial.println("");
    }
    }
  
void loop() {
if (WiFi.status() != WL_CONNECTED) {  //wifi not connected?
Serial.print("Connecting to ");
Serial.print(ssid);
Serial.println("...");
WiFi.begin(ssid, pass);
if (WiFi.waitForConnectResult() != WL_CONNECTED)
return;
Serial.println("WiFi connected");
}
if (WiFi.status() == WL_CONNECTED) {
//client object makes connection to server
if (!client.connected()) {
Serial.println("Connecting to MQTT server");
//Authenticating the client object
if (client.connect(MQTT::Connect("mqtt_client_name").set_auth(mqtt_user, mqtt_pass))) {
Serial.println("Connected to MQTT server");
} 
else 
{
Serial.println("Could not connect to MQTT server");   
}
}
if (client.connected())
client.loop();
SendEsp();
}
//SendTempHumid();  // this will send the dummy temparature reading
}
