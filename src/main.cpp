#include <Arduino.h>
#include "hw_timer.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WIFI SETTINGS
const char *ssid = "mySSID";
const char *password = "myKEY";

// MQTT Settings
const char *mqtt_server = "IPorDNS";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_password = "";
const char *mqtt_value_topic = "PWMController/value";

// PIN SETTINGS
const byte zcPin = 12;
const byte outPin = 13;

// OTHER SETTINGS
const byte mqttDebug = 0;

// Watchdog to avoid diverting undefinitely
long interval = 60000L;
long lastReceived = 0L;

byte tarValue = 0;
byte curValue = 0;
byte zcState = 0; // 0 = ready; 1 = processing;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ICACHE_RAM_ATTR dimTimerISR()
{
  curValue = tarValue;

  if (curValue == 0)
  {
    digitalWrite(outPin, 0);
  }
  else if (curValue == 255)
  {
    digitalWrite(outPin, 1);
  }
  else
  {
    digitalWrite(outPin, 1);
  }

  zcState = 0;
}

void ICACHE_RAM_ATTR zcDetectISR()
{
  if (zcState == 0)
  {
    zcState = 1;

    if (curValue < 255 && curValue > 0)
    {
      digitalWrite(outPin, 0);

      int dimDelay = 30 * (255 - curValue) + 400;
      hw_timer_arm(dimDelay);
    }
  }
}

void updateValue(int newValue)
{
  tarValue = newValue;
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  char c_payload[length];
  memcpy(c_payload, payload, length);
  c_payload[length] = '\0';

  String s_topic = String(topic);
  String s_payload = String(c_payload);

  if (mqttDebug)
  {
    Serial.print("MQTT in: ");
    Serial.print(s_topic);
    Serial.print(" = ");
    Serial.print(s_payload);
  }
  if (s_topic == mqtt_value_topic)
  {
    if (mqttDebug)
    {
      Serial.println("");
    }

    if (s_payload.toInt() != tarValue)
    {
      updateValue((byte)s_payload.toInt());
    }
  }
  else
  {
    if (mqttDebug)
    {
      Serial.println(" [unknown message]");
    }
  }
}

void mqttConnect() {

  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "PWMController-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("connected");

      Serial.print("MQTT topic '");
      Serial.print(mqtt_value_topic);
      if (mqttClient.subscribe(mqtt_value_topic))
      {
        Serial.println("': Subscribed");
      }
      else
      {
        Serial.print("': Failed");
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void)
{
  pinMode(zcPin, INPUT_PULLUP);
  pinMode(outPin, OUTPUT);

  digitalWrite(outPin, 0);

  Serial.begin(115200);
  Serial.println("");

  setup_wifi();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  attachInterrupt(zcPin, zcDetectISR, RISING);

  hw_timer_init(NMI_SOURCE, 0);
  hw_timer_set_func(dimTimerISR);
}

void loop(void)
{
  // PWM Watchdog
  long now = millis();
  if (tarValue > 0 && now - lastReceived > interval)
  {
    Serial.println("Message not received in the configured interval.");
    Serial.println("Resetting...");
    updateValue(0u);
    lastReceived = now;
  }

  if (!mqttClient.connected()) mqttConnect();
  mqttClient.loop();
}
