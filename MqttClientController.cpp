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

#include "MqttTopic.h"
#include <ConnectionMonitor.h>
#include <PubSubClientWrapper.h>
#include <MqttClientDbgCommand.h>
#include <MqttClientController.h>

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
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, (isMqttConnected ? "MQTT lib is connected" : "MQTT lib is disconnected"));
    }
    return isMqttConnected;
  }

  void notifyLanConnected(bool isLanConnected)
  {
    if (isLanConnected && (0 != m_mqttClientCtrl) && (0 != m_mqttClientCtrl->connMon()))
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
      if (m_mqttClientCtrl->getShallConnect() && !m_mqttClientCtrl->connMon()->isMqttConnected())
      {
        // possible workaround for a possible PubSubClient bug:
        m_mqttClientCtrl->mqttClientWrapper()->client().flush();

        const int nbrOfLoops = 10;
        for (int i = 0; i < nbrOfLoops; i++)
        {
          m_mqttClientCtrl->loop();
        }

        m_mqttClientCtrl->connect();
      }
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: OFF");
    }
  }

  void notifyMqttConnected(bool isMqttConnected)
  {
    if (isMqttConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "MQTT Connection: ON");

      // subscribe to topics
      MqttTopicSubscriber* msgHandlerChain = m_mqttClientCtrl->mqttSubscriberChain();
      if (0 != msgHandlerChain)
      {
        m_mqttClientCtrl->mqttSubscriberChain()->subscribe();
      }
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "MQTT Connection: OFF");
    }
  }
};

//-----------------------------------------------------------------------------

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
: m_shallConnect(false)
, m_trPortMqttctrl(new DbgTrace_Port("mqttctrl", DbgTrace_Level::info))
, m_connMon(new ConnectionMonitor(new MyConnMonAdapter(this)))
, m_mqttSubscriberChain(0)
{
  DbgCli_Topic* mqttClientTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "mqtt", "MQTT Client debug commands");
  new DbgCli_Cmd_MqttClientCon(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientDis(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientPub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientSub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientUnsub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientShow(mqttClientTopic, this);
}

MqttClientController::~MqttClientController()
{
  delete m_connMon->adapter();
  delete m_connMon;
  m_connMon = 0;
  setShallConnect(false);
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
  m_shallConnect = shallConnect;
  m_connMon->setMqttState(shallConnect);
  if (!shallConnect)
  {
    s_mqttClientWrapper->disconnect();
  }
}

bool MqttClientController::getShallConnect()
{
  return m_shallConnect;
}

void MqttClientController::connect()
{
  s_mqttClientWrapper->connect(WiFi.macAddress().c_str());
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
  if (m_connMon->isMqttConnected())
  {
    bool mqttIsConnected = s_mqttClientWrapper->loop();
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

int MqttClientController::subscribe(MqttTopicSubscriber* mqttSubscriber)
{
  addMqttSubscriber(mqttSubscriber);
  return s_mqttClientWrapper->subscribe(mqttSubscriber->getTopicString());
}

int MqttClientController::unsubscribe(const char* topic)
{
  // TODO: remove and delete the default handler object
  return s_mqttClientWrapper->unsubscribe(topic);
}

MqttTopicSubscriber* MqttClientController::mqttSubscriberChain()
{
  return m_mqttSubscriberChain;
}

void MqttClientController::addMqttSubscriber(MqttTopicSubscriber* mqttSubscriber)
{
  if (0 == m_mqttSubscriberChain)
  {
    m_mqttSubscriberChain = mqttSubscriber;
    Serial.print("MqttClientController::addMqttSubscriber(), added first mqttSubscriber: ");
  }
  else
  {
    m_mqttSubscriberChain->addMqttSubscriber(mqttSubscriber);
    Serial.print("MqttClientController::addMqttSubscriber(), added mqttSubscriber: ");
  }
  Serial.println(mqttSubscriber->getTopicString());
}
