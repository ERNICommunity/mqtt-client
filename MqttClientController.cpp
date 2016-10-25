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

#include <PubSubClientWrapper.h>
#include <MqttClientDbgCommand.h>

#include <MqttClientController.h>

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

class PubSubClientCallbackAdapter : public IMqttClientCallbackAdapter
{
public:
  void messageReceived(char* topic, byte* payload, unsigned int length)
  {
    char msg[length+1];
    memcpy(msg, payload, length);
    msg[length] = 0;
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));
    Serial.println(msg);
  }
};

//-----------------------------------------------------------------------------

const unsigned long mqttClientCtrlReconnectTimeMillis = 5000;

MqttClientController* MqttClientController::s_instance = 0;
IMqttClientWrapper* MqttClientController::s_mqttClientWrapper = 0;

MqttClientController* MqttClientController::Instance()
{
  if (0 == s_instance)
  {
    s_instance = new MqttClientController();
  }
  return s_instance;
}

MqttClientController::MqttClientController()
: m_reconnectTimer(new Timer(new MqttClientCtrlReconnectTimerAdapter(this), Timer::IS_RECURRING))
, m_isConnected(false)
{

  //-----------------------------------------------------------------------------
  // MQTT Client Commands
  //-----------------------------------------------------------------------------
  DbgCli_Topic* mqttClientTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "mqtt", "MQTT Client debug commands");
  new DbgCli_Cmd_MqttClientCon(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientDis(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientPub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientSub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientUnsub(mqttClientTopic, this);
}

MqttClientController::~MqttClientController()
{
  setShallConnect(false);

  delete m_reconnectTimer->adapter();
  m_reconnectTimer->attachAdapter(0);

  delete m_reconnectTimer;
  m_reconnectTimer = 0;
}

void MqttClientController::assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper)
{
  s_mqttClientWrapper = mqttClientWrapper;
  s_mqttClientWrapper->setCallbackAdapter(new PubSubClientCallbackAdapter());
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
    s_mqttClientWrapper->disconnect();
  }
}

bool MqttClientController::getShallConnect()
{
  return m_reconnectTimer->isRunning();
}

void MqttClientController::connect()
{
  const char* mqttClientId = "wiring-iot-skeleton";
  s_mqttClientWrapper->connect(mqttClientId);
}

void MqttClientController::reconnect()
{
  if (WiFi.isConnected())
  {
    Serial.println("LAN Client is connected");
    bool newIsConnected = s_mqttClientWrapper->connected();
    if (m_isConnected != newIsConnected)
    {
      // connection state changed
      m_isConnected = newIsConnected;

      delay(5000);

      int state = s_mqttClientWrapper->state();
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
      delay(5000);
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
  s_mqttClientWrapper->processMessages();
}

int MqttClientController::publish(const char* topic, const char* data)
{
  return s_mqttClientWrapper->publish(topic, data);
}

int MqttClientController::subscribe(const char* topic)
{
  return s_mqttClientWrapper->subscribe(topic);
}

int MqttClientController::unsubscribe(const char* topic)
{
  return s_mqttClientWrapper->unsubscribe(topic);
}

