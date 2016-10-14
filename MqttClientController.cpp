/*
 * MqttClientController.cpp
 *
 *  Created on: 13.10.2016
 *      Author: nid
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>
#include <Timer.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgCliCommand.h>
#include <DbgTraceContext.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <DbgPrintConsole.h>
#include <DbgTraceOut.h>
#include <MqttClientController.h>

//-----------------------------------------------------------------------------
// MQTT Client Dbg Cli Commands
//-----------------------------------------------------------------------------
class DbgCli_Cmd_MqttClientCon : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;

public:
  DbgCli_Cmd_MqttClientCon(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
  : DbgCli_Command(mqttClientTopic, "con", "Connect MQTT client to broker.")
  , m_mqttClient(mqttClient)
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    if (argc - idxToFirstArgToHandle > 0)
    {
      printUsage();
    }
    else
    {
      if (0 != m_mqttClient)
      {
        m_mqttClient->setShallConnect(true);
        Serial.println("MQTT client connecting to broker now.");
      }
    }
  }

  void printUsage()
  {
    Serial.println(getHelpText());
    Serial.println("Usage: dbg mqtt con");
  }
};

class DbgCli_Cmd_MqttClientDis : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;

public:
  DbgCli_Cmd_MqttClientDis(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
  : DbgCli_Command(mqttClientTopic, "dis", "Disconnect MQTT client from broker.")
  , m_mqttClient(mqttClient)
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    if (argc - idxToFirstArgToHandle > 0)
    {
      printUsage();
    }
    else
    {
      if (0 != m_mqttClient)
      {
        m_mqttClient->setShallConnect(false);
        Serial.println("MQTT client disconnecting from broker now.");
      }
    }
  }

  void printUsage()
  {
    Serial.println(getHelpText());
    Serial.println("Usage: dbg mqtt dis");
  }
};

class DbgCli_Cmd_MqttClientPub : public DbgCli_Command
{
private:
  MqttClientController* m_mqttClient;

public:
  DbgCli_Cmd_MqttClientPub(DbgCli_Topic* mqttClientTopic, MqttClientController* mqttClient)
  : DbgCli_Command(mqttClientTopic, "pub", "Publish a value to a topic using MQTT client.")
  , m_mqttClient(mqttClient)
  { }

  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
  {
    if (argc - idxToFirstArgToHandle != 2)
    {
      printUsage();
    }
    else
    {
      if (0 != m_mqttClient)
      {
        int retVal = m_mqttClient->publish(args[idxToFirstArgToHandle], args[idxToFirstArgToHandle+1]);
        Serial.print("MQTT client, publish ");
        Serial.println(retVal == 1 ? "successful" : "failed");
      }
    }
  }

  void printUsage()
  {
    Serial.println(getHelpText());
    Serial.println("Usage: dbg mqtt pub <topic> <value>");
  }
};

//-----------------------------------------------------------------------------

class MqttClientCtrlReconnectTimerAdapter : public TimerAdapter
{
private:
  MqttClientController* m_mqttClientCtrl;

public:
  MqttClientCtrlReconnectTimerAdapter(MqttClientController* mqttClientCtrl)
  : m_mqttClientCtrl(mqttClientCtrl)
  { }

  void timeExpired()
  {
    m_mqttClientCtrl->reconnect();
  }
};

//-----------------------------------------------------------------------------

const unsigned long mqttClientCtrlReconnectTimeMillis = 5000;

MqttClientController::MqttClientController(Client* lanClient, const char* mqttServerAddr, unsigned short int mqttPort)
: m_pubSubClient(new PubSubClient())
, m_reconnectTimer(new Timer(new MqttClientCtrlReconnectTimerAdapter(this), Timer::IS_RECURRING))
, m_isConnected(false)
{
  m_pubSubClient->setClient(*(lanClient));
  m_pubSubClient->setServer(mqttServerAddr, mqttPort);

  //-----------------------------------------------------------------------------
  // MQTT Client Commands
  //-----------------------------------------------------------------------------
  DbgCli_Topic* mqttClientTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "mqtt", "MQTT Client debug commands");
  new DbgCli_Cmd_MqttClientCon(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientDis(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientPub(mqttClientTopic, this);
}

MqttClientController::~MqttClientController()
{
  setShallConnect(false);

  delete m_reconnectTimer;
  m_reconnectTimer = 0;

  delete m_pubSubClient;
  m_pubSubClient = 0;
}

void MqttClientController::setShallConnect(bool shallConnect)
{
  if (shallConnect)
  {
    m_reconnectTimer->startTimer(mqttClientCtrlReconnectTimeMillis);
  }
  else
  {
    m_reconnectTimer->cancelTimer();
    m_pubSubClient->disconnect();
  }
}

bool MqttClientController::getShallConnect()
{
  return m_reconnectTimer->isRunning();
}

void MqttClientController::connect()
{
  const char* mqttClientId = "wiring-iot-skeleton";
  m_pubSubClient->connect(mqttClientId);
}

void MqttClientController::reconnect()
{
  if (WiFi.isConnected())
  {
    Serial.println("LAN Client is connected");
    bool newIsConnected = m_pubSubClient->connected();
    if (m_isConnected != newIsConnected)
    {
      // connection state changed
      m_isConnected = newIsConnected;

      int state = m_pubSubClient->state();
      Serial.print("MQTT client status: ");
      Serial.println(state == MQTT_CONNECTION_TIMEOUT      ? "CONNECTION_TIMEOUT"      :
                     state == MQTT_CONNECTION_LOST         ? "CONNECTION_LOST"         :
                     state == MQTT_CONNECT_FAILED          ? "CONNECT_FAILED"          :
                     state == MQTT_DISCONNECTED            ? "DISCONNECTED"            :
                     state == MQTT_CONNECTED               ? "CONNECTED"               :
                     state == MQTT_CONNECT_BAD_PROTOCOL    ? "CONNECT_BAD_PROTOCOL"    :
                     state == MQTT_CONNECT_BAD_CLIENT_ID   ? "CONNECT_BAD_CLIENT_ID"   :
                     state == MQTT_CONNECT_UNAVAILABLE     ? "CONNECT_UNAVAILABLE"     :
                     state == MQTT_CONNECT_BAD_CREDENTIALS ? "CONNECT_BAD_CREDENTIALS" :
                     state == MQTT_CONNECT_UNAUTHORIZED    ? "CONNECT_UNAUTHORIZED"    : "UNKNOWN");
    }

    if (m_isConnected)
    {
      // connected, subscribe to topics (if not yet done)
      Serial.println("MQTT connection ok");
    }
    else
    {
      // not connected, try to re-connect
      Serial.println("MQTT not connected - trying to connect");
      connect();
    }
  }
  else
  {
    Serial.println("LAN Client is not connected");
  }
}

void MqttClientController::loop()
{
  m_pubSubClient->loop();
}

int MqttClientController::publish(const char* topic, const char* data)
{
  return m_pubSubClient->publish(topic, data);
}
