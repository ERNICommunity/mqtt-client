# ERNI Community MQTT Client

MQTT Client with pluggable publish and subscribe topic objects

## Dependencies

- Arduino MQTT Client (https://github.com/256dpi/arduino-mqtt, Arduino Library Mgr: *MQTT*)
- ESP8266WiFi (ESP8266) or WiFi (ESP32)
- Spin Timer (3.0.0) (https://github.com/dniklaus/spin-timer, Arduino Library Mgr: *spin-timer*)
- Debug Trace (1.1.0) (https://github.com/ERNICommunity/dbg-trace, Arduino Library Mgr: *dbg-trace*)
- Debug CLI (1.3.0) (https://github.com/ERNICommunity/debug-cli, Arduino Library Mgr: *debug-cli*)

## Architecture

tbd

## Integration

### Arduino

#### Dependencies

Here are the additional dependencies the example sketch needs to be installed:

- SerialCommand (https://github.com/kroimon/Arduino-SerialCommand, Arduino Library Mgr: import ZIP)
- App Debug (2.0.1) (https://github.com/dniklaus/wiring-app-debug, Arduino Library Mgr: import ZIP)
- RamUtils (2.1.0) (https://github.com/dniklaus/arduino-utils-mem, Arduino Library Mgr: import ZIP)

#### Sketch

```C++
#include <Arduino.h>
#include <SerialCommand.h>
#include <SpinTimer.h>
#include <AppDebug.h>

#include <ECMqttClient.h>   // ERNI Community MQTT client wrapper library
#include <MqttTopic.h>

#define MQTT_SERVER "test.mosquitto.org"

SerialCommand* sCmd = 0;

//-----------------------------------------------------------------------------

void setupBuiltInLed()
{
#if defined(ESP8266)
  digitalWrite(LED_BUILTIN, 1);  // LED state is inverted on ESP8266
#else
  digitalWrite(LED_BUILTIN, 0);
#endif
}
 
void setBuiltInLed(bool state)
{
#if defined(ESP8266)
  digitalWrite(LED_BUILTIN, !state);  // LED state is inverted on ESP8266
#else
  digitalWrite(LED_BUILTIN, state);
#endif
}

//-----------------------------------------------------------------------------

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
      bool state = atoi(rxMsg->getRxMsgString());
      setBuiltInLed(state);
      // ... and marks the received message as handled (chain of responsibilities) 
      msgHasBeenHandled = true;
    }
  }
  return msgHasBeenHandled;
};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  setBuiltInLed(false);
  
  setupDebugEnv();

  //-----------------------------------------------------------------------------
  // ESP8266 / ESP32 WiFi Client
  //-----------------------------------------------------------------------------
  WiFi.mode(WIFI_STA);
    
  //-----------------------------------------------------------------------------
  // MQTT Client
  //-----------------------------------------------------------------------------
  ECMqttClient.begin(MQTT_SERVER);
  new TestLedMqttSubscriber();
}

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();           // process serial commands
  }
  ECMqttClient.loop();            // process MQTT Client
  scheduleTimers();               // process Timers
}

```

