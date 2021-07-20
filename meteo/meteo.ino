#include <SPI.h>
#include <Ethernet.h>
#include "LowPower.h"

// Choose a random mac address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "www.google.com"; // Server uning dns
EthernetClient client;

String city = "";
// This day
byte temp = 0; // Current temperature
String weather = ""; // Current weather
// This tomorrow
byte temp_max = 0; // Next day max temperature
byte temp_min = 0; // Next day min temperature
String weather_t = ""; // Next day temperature

void setup() {
  Serial.begin(1000000); // Show info

  // Start the Ethernet connection
  Serial.println("Initialize Ethernet with DHCP:");
  while (Ethernet.begin(mac) == 0) {
    Serial.println("  Failed to configure Ethernet using DHCP");
    Serial.println("  Reboot in 20 seconds");
    delay(20000);
  }
  Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() {
  // Start a connexion
  if (client.connect(server, 80)) {
    // Simple HTTP request to get weather
    client.println("GET /search?q=meteo HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
    // Initialize some variables
    bool detect_city = false;
    bool detect_temp = false;
    bool detect_city_end = false;

    // Pass some useless data
    for (byte i = 0; i < 16; i++) {
      while (!client.available()) delay(10);
      while (client.read() != '\n');
    }
    while (client.connected()) {
      if (client.available()) {
        char lire = client.read();
        // First filter
        if (detect_city_end == false && lire == 'k' && client.read() == 'C' && client.read() == 'r') {
          if (detect_city == false) detect_city = true;
          else {
            client.readStringUntil('>');
            client.readStringUntil('>');
            client.readStringUntil('>');
            city = client.readStringUntil('<');
            detect_city_end = true;
          }
        }
        // Second filter
        if (lire == 'i' && client.read() == ' ' && client.read() == 'A') {
          client.readStringUntil('>');
          if (detect_temp == false) detect_temp = true;
          else {
            temperature = (client.read() - 0x30) * 10;
            temperature += client.read() - 0x30;
            while (client.read() != '\n');
            weather = client.readStringUntil('<');
            break;
          }
        }
      }
    }
    // Stop client without reading next data
    client.stop();
  } else {
    Serial.println("Connection failed");
  }

  // Start new connection
  if (client.connect(server, 80)) {
    // Request
    client.println("GET /search?q=meteo+demain HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();

    while (!client.available()) delay(100);
    // Go to the start of the document
    while (client.read() != '<');
    // Pass 17 lines
    for (byte i = 0; i < 17; i++) {
      while (!client.available()) delay(100);
      while (client.read() != '\n');
    }
    delay(10);
    weather_t = client.readStringUntil('\n');
    client.readStringUntil(' ');
    temperature_max = (client.read() - 0x30) * 10;
    temperature_max += client.read() - 0x30;
    client.readStringUntil(':');
    client.read();
    temperature_min = (client.read() - 0x30) * 10;
    temperature_min += client.read() - 0x30;
    client.stop();
  } else {
    Serial.println("Connection failed");
  }

  // Use this function
  update_weather();
  // Update every 10 minutes
  LowPower.longPowerDown(600000L);
}


// Change this function
void update_weather(){
  Serial.println("+-----------------------------+");
  Serial.write('|');
  byte count = (29 - city.length())/2;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.print(city);
  count = (float)(29 - city.length())/2.0 + 0.5;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.println('|');
  Serial.println("+--------------+--------------+");
  Serial.println("|    Today     |   Tomorrow   |");
  Serial.print("|     ");
  if(temperature < 10) Serial.write(' ');
  Serial.print(temperature);
  Serial.print("°C     |  ");
  if(temperature_min < 10) Serial.write(' ');
  Serial.print(temperature_min);
  Serial.print("°C/");
  if(temperature_max < 10) Serial.write(' ');
  Serial.print(temperature_max);
  Serial.println("°C   |");
  Serial.write('|');
  count = (14 - weather.length())/2;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.print(weather);
  count = (float)(14 - weather.length())/2.0 + 0.5;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.write('|');
  count = (14 - weather_t.length())/2;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.print(weather_t);
  count = (float)(14 - weather_t.length())/2.0 + 0.5;
  for(byte i = 0; i < count; i++){
    Serial.write(' ');
  }
  Serial.println('|');
  Serial.println("+--------------+--------------+\n\n");
}
