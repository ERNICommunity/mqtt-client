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

#include <MqttMsgHandler.h>
#include <ConnectionMonitor.h>
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
    if (m_mqttClientCtrl->connMon()->isLanConnected())
    {
      TR_PRINT_STR(m_mqttClientCtrl->trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
      if (m_mqttClientCtrl->getShallConnect())
      {
        m_mqttClientCtrl->reconnect();
      }
    }
  }
};

//-----------------------------------------------------------------------------

class MyConnMonAdapter : public ConnMonAdapter
{
private:
  MqttClientController* m_mqttClientCtrl;

public:
  MyConnMonAdapter(MqttClientController* mqttClientCtrl)
  : ConnMonAdapter()
  , m_mqttClientCtrl(mqttClientCtrl)
  { }

  bool mqttConnectedRaw()
  {
    bool isMqttConnected = false;
    if (0 != m_mqttClientCtrl)
    {
      isMqttConnected = m_mqttClientCtrl->mqttClientWrapper()->connected();
    }
    return isMqttConnected;
  }

  void notifyLanConnected(bool isLanConnected)
  {
    if (isLanConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
      if (m_mqttClientCtrl->getShallConnect() && ! )
      {
        m_mqttClientCtrl->reconnect();
      }
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: OFF");
      m_mqttClientCtrl->
    }
  }

  void notifyMqttConnected(bool isMqttConnected)
  {
    if (isMqttConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "MQTT Connection: ON");
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "MQTT Connection: OFF");
    }
  }
};

//-----------------------------------------------------------------------------

const unsigned long mqttClientCtrlReconnectTimeMillis = 1000;

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
, m_trPortMqttctrl(new DbgTrace_Port("mqttctrl", DbgTrace_Level::info))
//, m_isConnected(false)
, m_connMon(new ConnectionMonitor(new MyConnMonAdapter(this)))
, m_handlerChain(0)
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
  delete m_connMon->adapter();
  delete m_connMon;
  m_connMon = 0;

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

IMqttClientWrapper* MqttClientController::mqttClientWrapper()
{
  return s_mqttClientWrapper;
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
  TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, "MQTT client status: ");
  TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, s_mqttClientWrapper->stateStr());

  if (m_connMon->isLanConnected())
  {
    TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, "LAN Client is connected");
//    m_isConnected = s_mqttClientWrapper->connected();

//    if (m_isConnected)
    if (m_connMon->isMqttConnected())
    {

      // connected, subscribe to topics (if not yet done)
      m_handlerChain->subscribe();
      TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, "MQTT connection ok");
    }
    else
    {
      // not connected, try to re-connect
      TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, "MQTT not connected - trying to connect");

      // possible workaround for a possible PubSubClient bug:
      s_mqttClientWrapper->disconnect();
      s_mqttClientWrapper->client().flush();

      loop();
      connect();
    }
  }
  else
  {
    TR_PRINT_STR(m_trPortMqttctrl, DbgTrace_Level::debug, "LAN Client is not connected");
    s_mqttClientWrapper->disconnect();
//    m_isConnected = false;
  }
}

ConnectionMonitor* MqttClientController::connMon()
{
  return m_connMon;
}

DbgTrace_Port* MqttClientController::trPort()
{
  return m_trPortMqttctrl;
}

void MqttClientController::loop()
{
//  if (m_isConnected)
  if (m_connMon->isMqttConnected())
  {
    bool mqttIsConnected = s_mqttClientWrapper->processMessages();
    m_connMon->setMqttState(mqttIsConnected);
  }
}

int MqttClientController::publish(const char* topic, const char* data)
{
  return s_mqttClientWrapper->publish(topic, data);
}

int MqttClientController::subscribe(const char* topic)
{
//  addMsgHandler(new DefaultMqttMsgHandler(topic));
  return s_mqttClientWrapper->subscribe(topic);
}

int MqttClientController::subscribe(MqttMsgHandler* mqttMsgHandler)
{
  addMsgHandler(mqttMsgHandler);
  return s_mqttClientWrapper->subscribe(mqttMsgHandler->getTopic());
}

int MqttClientController::unsubscribe(const char* topic)
{
  // TODO: remove and delete the default handler object
  return s_mqttClientWrapper->unsubscribe(topic);
}

MqttMsgHandler* MqttClientController::msgHandlerChain()
{
  return m_handlerChain;
}

void MqttClientController::addMsgHandler(MqttMsgHandler* handler)
{
  if (0 == m_handlerChain)
  {
    m_handlerChain = handler;
  }
  else
  {
    m_handlerChain->addHandler(handler);
  }
}
