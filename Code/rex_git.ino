#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#define DHTPIN 5     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// OLED Display
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define I2Cdisplay_SDA 33   //declaring pin for OLED
#define I2Cdisplay_SCL 32     //declaring pin for OLED

// LDR (Light Dependent Resistor)
#define ldr 35

const int duration = 1000;
const int led_pin = 12;

DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP280 bme; // I2C

TwoWire I2Cdisplay = TwoWire(1);

//Declaring OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cdisplay, -1);

// Replace with your network credentials
const char* ssid = "python";
const char* password = "python07";

// Initialize Telegram BOT
#define BOTtoken "1183444529:AAGkHiwjC-as0v4tWXVY0bcT8FDCwrlqpqw"  // your Bot Token (Get from Botfather)
#define CHAT_ID "1100403070"
//ronil chat id : "1100403070"
//group chat id : "-1001253214192"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void setup() {
  Serial.begin(115200);
  I2Cdisplay.begin(I2Cdisplay_SDA, I2Cdisplay_SCL, 100000);
  // Initialize OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  Serial.println(F("BMP280 test"));
  //handle led blink during new request...
  pinMode(led_pin, OUTPUT);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3D for 128x64
  delay(200);
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.print("Connecting...");
  display.println("\n\n\n\n\n");
  display.print(" Note: Turn ON WiFi. ");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  digitalWrite(led_pin, LOW);
  // Print ESP32 Local IP Address
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected!!! ");
  display.println("IP Address: ");
  display.print("    ");
  display.print(WiFi.localIP());
  display.display();
  delay(2000);
  display.clearDisplay();

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("Warning : ");
    display.println(" Check BMP280 Sensor ");
    display.display();
    while (1);
  }
  digitalWrite(led_pin, LOW);
  Serial.println(F("DHTxx test!"));
  dht.begin();

  // use any animation for OLED

}

void loop() {
  // Wait a few seconds between measurements.
  delay(500);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Read light intensity called Luminosity in %
  int ldrReading = map(analogRead(ldr), 0, 4095, 100, 0);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!!!"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("Warning : ");
    display.println(" Check DHT11 Sensor ");
    display.display();
    return;
  }
  //  // Compute heat index in Fahrenheit (the default)
  //  float hif = dht.computeHeatIndex(f, h);
  //  // Compute heat index in Celsius (isFahreheit = false)
  //  float hic = dht.computeHeatIndex(t, h, false);

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  disp();
}

void disp() {
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  float hic = dht.computeHeatIndex(t, h, false);
  int ldr1 = map(analogRead(ldr), 0, 4095, 100, 0);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Live Weather Forcast");
  display.println("_____________________");
  display.println("Temp : " + String(t) + " " + (char)247 + "C");
  display.println("Luminosity : " + String(ldr1) + " " + "%");
  display.println("Humi : " + String(h) + " %");
  display.println("HeatIndex : " + String(hic) + " " + (char)247 + "C");
  display.println("ATP : " + String(bme.readPressure()) + " Pa");
  display.println("Alt : " + String(bme.readAltitude(1013.25)) + " m");
  display.display();
}

void handleNewMessages(int numNewMessages)
{
  float t = dht.readTemperature();
  String temp = "🌡Temperature : " + String(t) + " °C";
  float f = dht.readTemperature(true);
  String tf = " or " + String(f) + " °F.\n";

  float h = dht.readHumidity();
  String humi = "⛅Humidity : " + String(h) + " %\n";

  float hic = dht.computeHeatIndex(t, h, false);
  String hi = "☀Heat Index : " + String(hic) + " °C\n";

  String alti = "⛰Approx Altitude : " + String(bme.readAltitude(1013.25)) + " m.\n";

  String pre = "⚗Pressure : " + String(bme.readPressure()) + " Pa.\n";
  
  // Read light intensity called Luminosity in %
  int ldrReading1 = map(analogRead(ldr), 0, 4095, 100, 0);
  String light = "🌤Luminosity : " + String(ldrReading1) + " %";

  String all = String(temp + tf + humi + hi + alti + pre + light);
  //
  for (int i = 0; i < numNewMessages ; i++) //para cada mensagem nova
  {
    String chat_id = String(bot.messages[i].chat_id); //id do chat
    String from_name = bot.messages[i].from_name;
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "🚫Unauthorized User.", "");
      continue;
    }

    String text = bot.messages[i].text; //texto que chegou
    if (text == "/start") {
      String snd = getCommands();
      bot.sendMessage(chat_id, snd, "");
    }
    else if (text == ".t")
    {
      bot.sendMessage(chat_id, String(temp + tf), "");
    }
    else if (text == ".h")
    {
      bot.sendMessage(chat_id, humi, "");
    }
    else if (text == ".al")
    {
      bot.sendMessage(chat_id, alti, "");
    }
    else if (text == ".p")
    {
      bot.sendMessage(chat_id, pre, "");
    }
    else if (text == ".hi")
    {
      bot.sendMessage(chat_id, hi, "");
    }
    else if (text == ".lu")
    {
      bot.sendMessage(chat_id, light, "");
    }
    else if (text == ".all") {
      bot.sendMessage(chat_id, all, "");
    }
    else
    {
      handleNotFound(chat_id);
    }
  }//for
}

void handleNotFound(String chat_id)
{
  String message = "Don't make me fool🤬\nUse /start to make conversion with me😤";

  bot.sendMessage(chat_id, message, "");
}

String getCommands()
{
  String from_name = bot.messages[0].from_name;
  String welcome = "Welcome, " + from_name + "😊\n";
  welcome += "📌Use following commands to get Live Weather Analysis📊.\n";
  welcome += "_.__________________________________._\n";
  welcome += "\t🔹 .t    Temprature🌡\n";
  welcome += "\t🔹 .h    Humidity⛅\n";
  welcome += "\t🔹 .al   Approx Altitude⛰\n";
  welcome += "\t🔹 .p    Pressure⚗\n";
  welcome += "\t🔹 .hi   Heat Index🌤\n";
  welcome += "\t🔸 .lu  - Luminosity🌤\n";
  welcome += "\t🔹 .all  🌡⛅⛰⚗🌤\n";
  welcome += "_.__________________________________._\n\n";
  welcome += "🛑 Please wait for approx 3 to 4 sec to measure data accuratly.";
  return welcome;
}
