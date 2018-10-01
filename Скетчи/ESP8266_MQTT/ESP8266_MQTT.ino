#include <ArduinoJson.h>
#include <MQTT.h>
#include <PubSubClient.h>
#include <PubSubClient_JSON.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#define ssid "WiFi"   
#define pass "10072003WiFi"   
//#define ssid "Zevs"   
//#define pass "R667fksX55fr"   
#define mqtt_server "m15.cloudmqtt.com"
#define mqtt_port 13810 
#define mqtt_user "ditozyzm"
#define mqtt_pass "v4D9ZUJydZ7c"
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
//Subscribe code
 if(Serial.read() == 000)
{
client.subscribe("Temp");
 client.subscribe("ControlTemp");
client.subscribe("Hum");
 client.subscribe("ControlHum");
client.subscribe("Day");
}
} else 
{
Serial.println("Could not connect to MQTT server");   
}
}
if (client.connected())
client.loop();
}
//SendTempHumid();  // this will send the dummy temparature reading
}
