#include "WiFi.h" // ESP32 WiFi include
#include "WiFiConfig.h" // My WiFi configuration
#include "time.h"
#include "PubSubClient.h"
#include <IRremote.h>
//#include "PinDefinitionsAndMore.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
const char* mqtt_server = "192.168.2.116";  /*MQTT Broker*/
char data[100];

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  900        /* Time ESP32 will go to sleep (in seconds) */
#define ONBOARD_LED  2

//The Remote codes

#define high  0xF700FF
#define low   0xF7807F
#define OFF   0xF740BF
#define ON    0xF7C03F
#define WHITE 0xF7E01F
#define flash 0xF7D02F
#define Strob 0xF7F00F
#define fade  0xF7C837
#define smuth 0xF7E817

// Red color column
#define RED           0xF720DF
#define bright_orange 0xF710EF  //From Red button +1 row
#define dark_orange   0xF730CF  //From Red button +2 row
#define dark_yellow   0xF708F7  //From Red button +3 row
#define bright_yellow 0xF728D7  //From Red button +4 row

//Green color column
#define GREEN         0xF7A05F
#define bright_turkis 0xF7906F  //From Green button +1 row
#define dark_turkis   0xF7B04F  //From Green button +2 row
#define cold_blue     0xF78877     //From Green button +3 row
#define warm_blue     0xF7A857     //From Green button +4 row

//Blue color column
#define BLUE        0xF7609F         
#define warm_purple 0xF750AF  //From Blue button +1 row
#define cold_purple 0xF7708F  //From Blue button +2 row
#define dark_pink   0xF748B7    //From Blue button +3 row
#define bright_pink 0xF76897  //From Blue button +4 row

/*MQTT client definitions */
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  
  // Set pin mode
  pinMode(ONBOARD_LED,OUTPUT);

  // put your setup code here, to run once:
  Serial.begin(9600);
  //delay(1000); //Take some time to open up the Serial Monitor
 
  ConnectToWiFi();

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  client.setServer(mqtt_server, 1883);  /*Inititate the MQTT server*/
  client.setCallback(callback);

 }

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S => 1");
}

void ConnectToWiFi()
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WiFiPassword);
  Serial.print("Connecting to "); Serial.println(SSID);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);

    if ((++i % 16) == 0)
    {
      Serial.println(F(" still trying to connect"));
    }
  }

  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());
} 

// MQTT Callback function

uint8_t sRepeats = 0;   //The repeats for the NEC protocol

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  int messageValue;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32_commands") {
    Serial.print("Code Recieved ");

     if (messageTemp == "WHITE"){
       messageValue = WHITE;      
    }
    
    if (messageTemp == "RED"){
       messageValue = RED;      
    }

    if (messageTemp == "BLUE"){
       messageValue = BLUE;      
    }

    if (messageTemp == "GREEN"){
       messageValue = GREEN;  
    }

    if (messageTemp == "low"){
       messageValue = low;
    }

    if (messageTemp == "high"){
       messageValue = high;
    }

    if (messageTemp == "ON"){
       messageValue = ON;
    }

    if (messageTemp == "OFF"){
       messageValue = OFF;
    }

    if (messageTemp == "fade"){
       messageValue = fade;
    }

    if (messageTemp == "smuth"){
       messageValue = smuth;
    }

    if (messageTemp == "Strob"){
       messageValue = Strob;
    }

    if (messageTemp == "flash"){
       messageValue = flash;
    }   
 
    Serial.print(messageValue);
    
    IrSender.sendNEC(messageValue, sRepeats);
    // Blink as acknwoledgement
    digitalWrite(ONBOARD_LED, HIGH);
    delay(1000);
    digitalWrite(ONBOARD_LED, LOW);
    }
  }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32_commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop() {

if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
