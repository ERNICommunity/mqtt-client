# ERNI Community MQTT Client



MQTT Client with pluggable publish and subscribe topic objects

## Architecture



## Integration

Arduino Sketch:

```C++
#include <ECMqttClient.h>   // ERNI Community MQTT client wrapper library (depends on Arduino MQTT library)
#include <MqttTopic.h>

#define MQTT_SERVER "test.mosquitto.org"

WiFiClient* wifiClient = 0;

class TestLedMqttSubscriber : public MqttTopicSubscriber
{
public:
  TestLedMqttSubscriber()
  : MqttTopicSubscriber("test/led")
  { }

  virtual ~TestLedMqttSubscriber()
  { }

  bool processMessage(MqttRxMsg* rxMsg)
  {
    bool msgHasBeenHandled = false;
    
    if (0 != rxMsg)
    {
      // this subscriber object takes the responsibility
      bool pinState = atoi(rxMsg->getRxMsgString());
      digitalWrite(LED_BUILTIN, !pinState);  // LED state is inverted on ESP8266
      // ... and marks the received message as handled (chain of responsibilities) 
      msgHasBeenHandled = true;
    }
  }
  return msgHasBeenHandled;
};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  // set LED off
  digitalWrite(LED_BUILTIN, 1);  // LED state is inverted on ESP8266

  //-----------------------------------------------------------------------------
  // ESP8266 / ESP32 WiFi Client
  //-----------------------------------------------------------------------------
  WiFi.mode(WIFI_STA);
  wifiClient = new WiFiClient();
    
  //-----------------------------------------------------------------------------
  // MQTT Client
  //-----------------------------------------------------------------------------
  ECMqttClient.begin(MQTT_SERVER);
  new TestLedMqttSubscriber();

}

void loop()
{
  ECMqttClient.loop();            // process MQTT Client
}

```

