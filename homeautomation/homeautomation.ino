#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//#define WATERPUMP 2  
#define TRIG 5
#define ECHO 4
#define SOILSENSOR 2
//#define RELAY 3
//#define BUZZ 2

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "test"
#define WLAN_PASS       "test12345"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "SumanWahengbam"
#define AIO_KEY         "42fdade34ccf474a9bf6d13890d8aa8f"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay");
Adafruit_MQTT_Subscribe waterpump = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/waterpump");

Adafruit_MQTT_Publish soil = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soilsensor");
Adafruit_MQTT_Publish dis = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ultrasonicdata");

/*************************** Sketch Code ************************************/
void MQTT_connect();
int Ulra_sensor();
int distance;
double duration;
void setup() 
{
  //pinMode(WATERPUMP, OUTPUT);
  pinMode(TRIG, OUTPUT);
  //pinMode(RELAY,OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(SOILSENSOR, INPUT);
 // pinMode(BUZZ,OUTPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println(F("HOME AUTOMATION DEMO"));
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  // Setup MQTT subscription
  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&waterpump);
}
uint32_t x=0;
void loop() 
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  //delay(1000);
  int d = Ulra_sensor(TRIG,ECHO);

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) 
  {
    // Check if its the onoff button feed
    if (subscription == &onoffbutton) 
    {      
      if (strcmp((char *)onoffbutton.lastread, "ON") == 0) 
      {
        //digitalWrite(RELAY, LOW);
        Serial.println("Light ON");
      }
      if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) 
      {
        //digitalWrite(RELAY, HIGH);
        Serial.println("Light off"); 
      }
    }
    if (subscription == &waterpump) 
    {   
      if (strcmp((char *)waterpump.lastread, "ON") == 0) 
      {
        //digitalWrite(WATERPUMP, HIGH);
        Serial.println("Water Pump ON");
      }
      if (strcmp((char *)waterpump.lastread, "OFF") == 0)
      {
        //digitalWrite(WATERPUMP, LOW);
        Serial.println("Water Pump OFF"); 
      }
    }
  }
  //publish
  if (! dis.publish(String(d).c_str())) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    if (d<20)
    { 
      //digitalWrite(BUZZ,HIGH);
      Serial.println("INTRUDER DETECTED BUZZER ON");
    }
    else if(d>20)
    {
      //digitalWrite(BUZZ,LOW);
      Serial.println("INTRUDER NOT DETECTED BUZZER OFF"); 
    }
  }
//  if (! soil.publish(String(s).c_str())) {
//    Serial.println(F("Failed"));
//  } else {
//    Serial.println(F("OK!"));
//  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

int Ulra_sensor(int trig, int echo)
{
  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
  duration = pulseIn(echo,HIGH);
  distance = 0.0343 *duration/2;
  return distance;
}
