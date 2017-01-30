#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET


// Here's how to control the LEDs from any two pins:
//#define DATAPIN    4
//#define CLOCKPIN   0
//Adafruit_DotStar strip = Adafruit_DotStar(
//NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#define FRONT 0
#define FRENCH 1
#define KITCHEN 2
#define MUSIC 3
#define PRAYER 4
#define NUMDOORS  5

#define NUMPIXELS 5 // Number of LEDs in strip
#define BUZZER 4

#define NIGHTMODE 14 // white
#define DATAPIN   2 
#define CLOCKPIN  0
Adafruit_DotStar strip = Adafruit_DotStar(
                           NUMPIXELS, DATAPIN, CLOCKPIN); // g r b is the native order

// Update these with values suitable for your network.
const char* ssid = "pvr";
const char* password = "Life is Good";
const char* mqtt_server = "192.168.1.2";

/*******************************************************************
  so i think we will have an array of state machines. also, an array
  of structs or something to hold state like when to next do
  something for each door / pixel, what color the pixel is, etc.

*******************************************************************/
bool buzzing = false;
bool isNightMode = false;
unsigned long blinkDelay = 500UL;
typedef struct
{
  unsigned long lastTime;
  uint32_t color;
  bool currentOutput;
  bool isBlinking;
  bool isOpen;
} door;

door doors[NUMDOORS] = {
  {0, 0x00FF00, false, false, false}, //red
  {0, 0xFFFF00, false, false, false}, //yellow
  {0, 0x800000, false, false, false}, //green
  {0, 0x0000FF, false, false, false}, //blue
  {0, 0x008080, false, false, false} //purple
};

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char buff[length + 1];
  for (int i = 0; i < length; i++) {
    buff[i] = (char)payload[i];
  }
  buff[length] = '\0';
  int num = atoi(buff);

  for (int i = 0; i < NUMDOORS; i++) {
    doors[i].isOpen = (bool)(num & 1 << i);
  }
}

void setInputsAndOutputs() {
  isNightMode = digitalRead(NIGHTMODE);
  buzzing = false;
  for (int i = 0; i < NUMDOORS; i++) {
    if (!isNightMode){//not nightMode
      doors[i].isBlinking = false;
    }
    if (doors[i].isOpen) {
      if (isNightMode) {
        doors[i].isBlinking = true;
      }
      buzzing = true;
      doors[i].currentOutput = true;
      strip.setPixelColor(i, doors[i].color);
    } else {//not open
      if (doors[i].isBlinking) {
        if ((millis() - doors[i].lastTime) > blinkDelay) {
          doors[i].lastTime = millis();
          if (doors[i].currentOutput) {//blink off
            strip.setPixelColor(i, 0);
          } else {//blink on
            strip.setPixelColor(i, doors[i].color);
          }
          doors[i].currentOutput = !doors[i].currentOutput;
        }
      } else {//not blinking, not open, turn off pixel
         strip.setPixelColor(i, 0);
         doors[i].currentOutput = false;
      }
    }
  }
  digitalWrite(BUZZER, buzzing);
  strip.show();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("doors");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(NIGHTMODE, INPUT);

  strip.begin(); // Initialize pins for output
  strip.setBrightness(10);
  strip.show();  // Turn all LEDs off ASAP

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  setInputsAndOutputs();
  client.loop();
  /*
    long now = millis();
    if (now - lastMsg > 2000) {
      lastMsg = now;
      ++value;
      snprintf (msg, 75, "hello world #%ld", value);
      //client.publish("outTopic", msg);
    }*/
}
