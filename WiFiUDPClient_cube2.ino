#include <WiFi.h>
#include <NetworkUdp.h>

// WiFi network name and password:
const char *networkName = "PichotLiving";
const char *networkPswd = "22675623";

//IP address to send UDP data to:
const char *udpAddress = "192.168.10.181";
const int udpPort = 9000;

#define X 0
#define Y 1
#define WIDTH 800
#define HEIGHT 600
#define CONFIG 9
#define CUBE_SIZE 50
#define READS 100

char config = 0;
unsigned int x, y;
unsigned int tempX, tempY;
unsigned int minX = 4095;
unsigned int maxX = 2492;
unsigned int minY = 4095;
unsigned int maxY = 2456;
unsigned int realMidX = 3294;
unsigned int realMidY = 3276;
unsigned int offsetX = 256 + CUBE_SIZE;
unsigned int offsetY = 253 + CUBE_SIZE;
unsigned long timer = millis();

//Are we currently connected?
boolean connected = false;
boolean _config = false;

//The udp library class
NetworkUDP udp;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CONFIG, INPUT_PULLUP);
  blink(3);

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);

  Serial.println("For calibrate press the boton...");
  delay(2000);
  if ((digitalRead(CONFIG) == 0)) {
    Serial.println("Ready for calibration");
    delay(2000);
    timer = millis();

    while (config < 5) {
      if (!_config) {
        switch (config) {
          case 0:
            minX = analogRead(X);
            Serial.print("minX:");
            Serial.println(minX);
            _config = true;
            break;
          case 1:
            maxX = analogRead(X);
            Serial.print("maxY:");
            Serial.println(maxX);
            _config = true;
            break;
          case 2:
            minY = analogRead(Y);
            Serial.print("minY:");
            Serial.println(minY);
            _config = true;
            break;
          case 3:
            maxY = analogRead(Y);
            Serial.print("maxY");
            Serial.println(maxY);
            _config = true;
            break;
          case 4:
            realMidX = analogRead(X);
            realMidY = analogRead(Y);
            Serial.print("Center: ");
            Serial.print(realMidX);
            Serial.print(",");
            Serial.println(realMidY);
            _config = true;
            break;
        }
      }
      if (millis() - timer > 2000) {
        blink(1);
        config++;
        timer = millis();
        _config = false;
      }
    }
    offsetX = (realMidX - (minX - ((minX - maxX) / 2)));
    offsetY = (realMidY - (minY - ((minY - maxY) / 2)));
  }
  Serial.println("RUN");



  Serial.print("minX:");
  Serial.println(minX);
  Serial.print("maxX:");
  Serial.println(maxX);
  Serial.print("minY:");
  Serial.println(minY);
  Serial.print("maxY:");
  Serial.println(maxY);
  Serial.print("midX:");
  Serial.println(realMidX);
  Serial.print("midY:");
  Serial.println(realMidY);
  Serial.print("offSetX:");
  Serial.println(offsetX);
  Serial.print("offsetY:");
  Serial.println(offsetY);
}

void loop() {
  //only send data when connected
  if (connected) {
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
    
    //reading average
    tempX = 0;
    tempY = 0;
    for (int i = 0; i < READS; i++) {
      tempX += analogRead(X);
      tempY += analogRead(Y);
      delay(1);
    }
    tempX = tempX / READS;
    tempY = tempY / READS;

    x = map(tempX - offsetX, minX - offsetX, maxX, 0, WIDTH - CUBE_SIZE);
    y = map(tempY - offsetY, minY - offsetY, maxY, 0, HEIGHT - CUBE_SIZE);
    if (x < 1) x = 0;
    if (y < 1) y = 0;
    if (x > (WIDTH - CUBE_SIZE - 1)) x = WIDTH - CUBE_SIZE;
    if (y > (HEIGHT - CUBE_SIZE - 1)) y = HEIGHT - CUBE_SIZE;

    char buffer[20];

    //x = x (WIDTH/2)
    sprintf(buffer, "%u,%u", x, y);
    udp.printf(buffer);
    udp.endPacket();
  }

  // Serial.print(tempX);
  // Serial.print(", ");
  // Serial.print(x);
  // Serial.print("-");
  // Serial.print(tempY);
  // Serial.print(", ");
  // Serial.println(y);

  delay(20);
}

void connectToWiFi(const char *ssid, const char *pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);  // Will call WiFiEvent() from another thread.

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

// WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      //initializes the UDP state

      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
    default: break;
  }
}

void blink(char _times) {
  for (char i; i < _times; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
}