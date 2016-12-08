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

#include <LanConnectionMonitor.h>
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

class MyLanConnMonAdapter : public LanConnMonAdapter
{
private:
  MqttClientController* m_mqttClientCtrl;

public:
  MyLanConnMonAdapter(MqttClientController* mqttClientCtrl)
  : LanConnMonAdapter()
  , m_mqttClientCtrl(mqttClientCtrl)
  { }

  void notifyLanConnected(bool isLanConnected)
  {
    if (isLanConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
      if (m_mqttClientCtrl->getShallConnect())
      {
        m_mqttClientCtrl->reconnect();
      }
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: OFF");
    }
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
, m_lanConnMon(new LanConnectionMonitor(new MyLanConnMonAdapter(this)))
, m_isConnected(false)
, m_trPort(new DbgTrace_Port("mqttctrl", DbgTrace_Level::info))
{
  DbgCli_Topic* mqttClientTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "mqtt", "MQTT Client debug commands");
  new DbgCli_Cmd_MqttClientCon(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientDis(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientPub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientSub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientUnsub(mqttClientTopic, this);
}

MqttClientController::~MqttClientController()
{
  delete m_lanConnMon->adapter();
  delete m_lanConnMon;
  m_lanConnMon = 0;
  delete m_reconnectTimer;
  m_reconnectTimer = 0;

  setShallConnect(false);

  delete m_reconnectTimer->adapter();
  m_reconnectTimer->attachAdapter(0);

  delete m_reconnectTimer;
  m_reconnectTimer = 0;
}

void MqttClientController::assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper, IMqttClientCallbackAdapter* mqttClientCallbackAdapter)
{
  s_mqttClientWrapper = mqttClientWrapper;
  s_mqttClientWrapper->setCallbackAdapter(mqttClientCallbackAdapter);
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
  if (m_lanConnMon->isConnected())
  {
    TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "LAN Client is connected");
    bool newIsConnected = s_mqttClientWrapper->connected();
    if (m_isConnected != newIsConnected)
    {
      // connection state changed
      m_isConnected = newIsConnected;

//      delay(5000);

      int state = s_mqttClientWrapper->state();
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "MQTT client status: ");
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, (state == MQTT_CONNECTION_TIMEOUT      ? "  CONNECTION_TIMEOUT"      :
                                                     state == MQTT_CONNECTION_LOST         ? "  CONNECTION_LOST"         :
                                                     state == MQTT_CONNECT_FAILED          ? "  CONNECT_FAILED"          :
                                                     state == MQTT_DISCONNECTED            ? "  DISCONNECTED"            :
                                                     state == MQTT_CONNECTED               ? "  CONNECTED"               :
                                                     state == MQTT_CONNECT_BAD_PROTOCOL    ? "  CONNECT_BAD_PROTOCOL"    :
                                                     state == MQTT_CONNECT_BAD_CLIENT_ID   ? "  CONNECT_BAD_CLIENT_ID"   :
                                                     state == MQTT_CONNECT_UNAVAILABLE     ? "  CONNECT_UNAVAILABLE"     :
                                                     state == MQTT_CONNECT_BAD_CREDENTIALS ? "  CONNECT_BAD_CREDENTIALS" :
                                                     state == MQTT_CONNECT_UNAUTHORIZED    ? "  CONNECT_UNAUTHORIZED"    : "  UNKNOWN"));
    }

    loop();

    if (m_isConnected)
    {
      // connected, subscribe to topics (if not yet done)
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "MQTT connection ok");
    }
    else
    {
      // not connected, try to re-connect
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "MQTT not connected - trying to connect");
//      delay(5000);
      connect();
    }
  }
  else
  {
    TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "LAN Client is not connected");
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

