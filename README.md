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

### Arduino Framework

The following example shows the integration of this component into an Arduino Framework based application.

#### Dependencies

Here are the additional dependencies the example sketch needs to be installed:

- SerialCommand (https://github.com/kroimon/Arduino-SerialCommand, Arduino Library Mgr: import ZIP)
- App Debug (2.0.1) (https://github.com/dniklaus/wiring-app-debug, Arduino Library Mgr: import ZIP)
- RamUtils (2.1.0) (https://github.com/dniklaus/arduino-utils-mem, Arduino Library Mgr: import ZIP)

#### Example Sketch

```C++
#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
// see https://github.com/espressif/arduino-esp32/issues/1960#issuecomment-429546528
#endif
#include <SerialCommand.h>
#include <SpinTimer.h>
#include <AppDebug.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgCliCommand.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>

#include <ECMqttClient.h>   // ERNI Community MQTT client wrapper library
#include <MqttTopic.h>

#define MQTT_SERVER "test.mosquitto.org"

SerialCommand* sCmd = 0;

//-----------------------------------------------------------------------------
// ESP8266 / ESP32 WiFi Client
//-----------------------------------------------------------------------------
#if defined(ESP8266) || defined(ESP32)
WiFiClient wifiClient;
#endif

//-----------------------------------------------------------------------------

void setBuiltInLed(bool state)
{
#if defined(ESP8266)
  digitalWrite(LED_BUILTIN, !state);  // LED state is inverted on ESP8266
#else
  digitalWrite(LED_BUILTIN, state);
#endif
}


//-----------------------------------------------------------------------------
// WiFi Commands
//-----------------------------------------------------------------------------
class DbgCli_Cmd_WifiMac : public DbgCli_Command
{
public:
  DbgCli_Cmd_WifiMac(DbgCli_Topic* wifiTopic)
  : DbgCli_Command(wifiTopic, "mac", "Print MAC Address.")
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    Serial.println();
    Serial.print("Wifi MAC: ");
    Serial.println(WiFi.macAddress().c_str());
    Serial.println();
  }
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_WifiNets : public DbgCli_Command
{
public:
  DbgCli_Cmd_WifiNets(DbgCli_Topic* wifiTopic)
  : DbgCli_Command(wifiTopic, "nets", "Print nearby networks.")
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    bool bailOut = false;

    // scan for nearby networks:
    Serial.println();
    Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
      Serial.println("Couldn't get a wifi connection");
      bailOut = true;
    }

    if (!bailOut)
    {
      // print the list of networks seen:
      Serial.print("number of available networks:");
      Serial.println(numSsid);

      // print the network number and name for each network found:
      for (int thisNet = 0; thisNet < numSsid; thisNet++)
      {
        Serial.print(thisNet);
        Serial.print(") ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print(" - Signal: ");
        Serial.print(WiFi.RSSI(thisNet));
        Serial.print(" dBm");
        Serial.print(" - Encryption: ");
        printEncryptionType(WiFi.encryptionType(thisNet));
      }
    }
    Serial.println();
  }
private:
  void printEncryptionType(int thisType)
  {
    // read the encryption type and print out the name:
    switch (thisType) {
#if ! defined(ESP32)
// TODO: solve this for ESP32!
      case ENC_TYPE_WEP:
        Serial.println("WEP");
        break;
      case ENC_TYPE_TKIP:
        Serial.println("WPA");
        break;
      case ENC_TYPE_CCMP:
        Serial.println("WPA2");
        break;
      case ENC_TYPE_NONE:
        Serial.println("None");
        break;
      case ENC_TYPE_AUTO:
        Serial.println("Auto");
        break;
#endif
      default:
        Serial.println("Unknown");
        break;
    }
  }
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_WifiStat : public DbgCli_Command
{
public:
  DbgCli_Cmd_WifiStat(DbgCli_Topic* wifiTopic)
  : DbgCli_Command(wifiTopic, "stat", "Show WiFi connection status.")
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    wl_status_t wlStatus = WiFi.status();
    Serial.println();
    Serial.println(wlStatus == WL_NO_SHIELD       ? "NO_SHIELD      " :
                   wlStatus == WL_IDLE_STATUS     ? "IDLE_STATUS    " :
                   wlStatus == WL_NO_SSID_AVAIL   ? "NO_SSID_AVAIL  " :
                   wlStatus == WL_SCAN_COMPLETED  ? "SCAN_COMPLETED " :
                   wlStatus == WL_CONNECTED       ? "CONNECTED      " :
                   wlStatus == WL_CONNECT_FAILED  ? "CONNECT_FAILED " :
                   wlStatus == WL_CONNECTION_LOST ? "CONNECTION_LOST" :
                   wlStatus == WL_DISCONNECTED    ? "DISCONNECTED   " : "UNKNOWN");
    Serial.println();
    WiFi.printDiag(Serial);
    Serial.println();
  }
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_WifiDis : public DbgCli_Command
{
public:
  DbgCli_Cmd_WifiDis(DbgCli_Topic* wifiTopic)
  : DbgCli_Command(wifiTopic, "dis", "Disconnect WiFi.")
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    Serial.println();
    if (argc - idxToFirstArgToHandle > 0)
    {
      printUsage();
    }
    else
    {
      const bool DO_NOT_SET_wifioff = false;
      WiFi.disconnect(DO_NOT_SET_wifioff);
      Serial.println("WiFi is disconnected now.");
    }
    Serial.println();
  }

  void printUsage()
  {
    Serial.println(getHelpText());
    Serial.println("Usage: dbg wifi dis");
  }
};

//-----------------------------------------------------------------------------

class DbgCli_Cmd_WifiCon : public DbgCli_Command
{
public:
  DbgCli_Cmd_WifiCon(DbgCli_Topic* wifiTopic)
  : DbgCli_Command(wifiTopic, "con", "Connect to WiFi.")
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    Serial.println();
   if (argc - idxToFirstArgToHandle != 2)
    {
      printUsage();
    }
    else
    {
      const char* ssid = args[idxToFirstArgToHandle];
      const char* pass = args[idxToFirstArgToHandle+1];
      Serial.print("SSID: ");
      Serial.print(ssid);
      Serial.print(", pass: ");
      Serial.println(pass);
      WiFi.begin(ssid, pass);
      Serial.println("WiFi is connecting now.");
    }
   Serial.println();
  }

  void printUsage()
  {
    Serial.println(getHelpText());
    Serial.println("Usage: dbg wifi con <SSID> <passwd>");
  }
};


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
    return msgHasBeenHandled;
  }
};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  setBuiltInLed(false);

  Serial.begin(9600);
  setupDebugEnv();

  //-----------------------------------------------------------------------------
  // WiFi Commands
  //-----------------------------------------------------------------------------
#if defined(ESP8266) || defined(ESP32)
  DbgCli_Topic* wifiTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "wifi", "WiFi debug commands");
  new DbgCli_Cmd_WifiMac(wifiTopic);
  new DbgCli_Cmd_WifiNets(wifiTopic);
  new DbgCli_Cmd_WifiStat(wifiTopic);
  new DbgCli_Cmd_WifiDis(wifiTopic);
  new DbgCli_Cmd_WifiCon(wifiTopic);
#endif

  //-----------------------------------------------------------------------------
  // ESP8266 / ESP32 WiFi Client
  //-----------------------------------------------------------------------------
  WiFi.mode(WIFI_STA);
    
  //-----------------------------------------------------------------------------
  // MQTT Client
  //-----------------------------------------------------------------------------
  ECMqttClient.begin(MQTT_SERVER, ECMqttClientClass::defaultMqttPort, wifiClient, WiFi.macAddress().c_str());
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

