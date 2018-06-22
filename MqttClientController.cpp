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

#include <MqttTopic.h>
#include <ConnectionMonitor.h>
#include <PubSubClientWrapper.h>
#include <MqttMockClientWrapper.h>
#include <MqttClientWrapper.h>
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

  bool appProtocolConnectedRaw()
  {
    bool isMqttConnected = false;
    if (0 != m_mqttClientCtrl)
    {
      isMqttConnected = m_mqttClientCtrl->mqttClientWrapper()->connected();
      TR_PRINTF(trPort(), DbgTrace_Level::debug, "MQTT client is %s", isMqttConnected ? "connected" : "disconnected");
    }
    return isMqttConnected;
  }

  bool shallAppProtocolConnect()
  {
    bool shallMqttConnect = false;
    if (0 != m_mqttClientCtrl)
    {
      shallMqttConnect = m_mqttClientCtrl->getShallConnect();
      TR_PRINTF(trPort(), DbgTrace_Level::debug, "MQTT client shall %s", shallMqttConnect ? "connect" : "not connect");
    }
    return shallMqttConnect;
  }

  void actionConnectAppProtocol()
  {
    bool isConnected = false;
    if (0 != m_mqttClientCtrl)
    {
      TR_PRINTF(trPort(), DbgTrace_Level::error, "actionConnectAppProtocol()");
      isConnected = m_mqttClientCtrl->connect();
      TR_PRINTF(trPort(), DbgTrace_Level::error, "actionConnectAppProtocol() done - MQTT client %s; %s",
          isConnected ? "connected" : "NOT connected",
          m_mqttClientCtrl->mqttClientWrapper()->connected() ? "connected" : "NOT connected");
    }
  }

  void notifyLanConnected(bool isLanConnected)
  {
    if (isLanConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: OFF");
    }
  }

  void notifyAppProtocolConnected(bool isMqttConnected)
  {
    if (isMqttConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::info, "MQTT Connection: ON");

      // subscribe to topics
      MqttTopicSubscriber* subscriberChain = m_mqttClientCtrl->mqttSubscriberChain();
      if (0 != subscriberChain)
      {
        TR_PRINTF(trPort(), DbgTrace_Level::debug, "notifyAppProtocolConnected(), subscriberChain->subscribe()");
        subscriberChain->subscribe();
        TR_PRINTF(trPort(), DbgTrace_Level::info, "notifyAppProtocolConnected(), subscriberChain->subscribe() done");
      }

      // publish the auto publisher topics
      MqttTopicPublisher* publisherChain = m_mqttClientCtrl->mqttPublisherChain();
      if (0 != publisherChain)
      {
        TR_PRINTF(trPort(), DbgTrace_Level::debug, "notifyAppProtocolConnected(), publisherChain->publishAll()");
        publisherChain->publishAll();
        TR_PRINTF(trPort(), DbgTrace_Level::info, "notifyAppProtocolConnected(), publisherChain->publishAll() done");
      }
    }
    else
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::info, "MQTT Connection: OFF");
    }
  }
};

//-----------------------------------------------------------------------------

MqttClientController* MqttClientController::s_instance = 0;
IMqttClientWrapper* MqttClientController::s_mqttClientWrapper = 0;
const unsigned short int MqttClientController::defaultMqttPort = 1883;

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
, m_connMon(new ConnMon(new MyConnMonAdapter(this)))
, m_mqttSubscriberChain(0)
, m_mqttPublisherChain(0)
{
  DbgCli_Topic* mqttClientTopic = new DbgCli_Topic(DbgCli_Node::RootNode(), "mqtt", "MQTT Client debug commands");
  new DbgCli_Cmd_MqttClientCon(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientDis(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientPub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientSub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientUnsub(mqttClientTopic, this);
  new DbgCli_Cmd_MqttClientShow(mqttClientTopic, this);

  Client* lanClient = 0;
#ifdef ESP8266
  lanClient = new WiFiClient();
#endif
  if (0 != lanClient)
  {
//    assignMqttClientWrapper(new PubSubClientWrapper(*(lanClient), "iot.eclipse.org"), new PubSubClientCallbackAdapter());
//    assignMqttClientWrapper(new MqttMockClientWrapper(*(lanClient), "test.mosquitto.org"), new MqttMockClientCallbackAdapter());
    assignMqttClientWrapper(new MqttClientWrapper(*(lanClient), "test.mosquitto.org"), new MqttClientCallbackAdapter());
    setShallConnect(true);
  }
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
  if (0 != s_mqttClientWrapper)
  {
    s_mqttClientWrapper->setCallbackAdapter(mqttClientCallbackAdapter);
  }
}


IMqttClientWrapper* MqttClientController::mqttClientWrapper()
{
  return s_mqttClientWrapper;
}

void MqttClientController::setServer(const char* domain, uint16_t port)
{
  if (0 != s_mqttClientWrapper)
  {
    s_mqttClientWrapper->setServer(domain, port);
  }
}

void MqttClientController::setClient(Client& client)
{
  if (0 != s_mqttClientWrapper)
  {
    s_mqttClientWrapper->setClient(client);
  }
}


void MqttClientController::setShallConnect(bool shallConnect)
{
  m_shallConnect = shallConnect;
  if (!shallConnect)
  {
    if (0 != s_mqttClientWrapper)
    {
      s_mqttClientWrapper->disconnect();
    }
  }
}

bool MqttClientController::getShallConnect()
{
  return m_shallConnect;
}

bool MqttClientController::connect()
{
  bool isConnected = false;
  if (0 != s_mqttClientWrapper)
  {
    isConnected = s_mqttClientWrapper->connect(WiFi.macAddress().c_str());
  }
  return isConnected;
}

ConnMon* MqttClientController::connMon()
{
  return m_connMon;
}

DbgTrace_Port* MqttClientController::trPort()
{
  return m_trPortMqttctrl;
}

void MqttClientController::loop()
{
  if (0 != m_connMon)
  {
    if (m_connMon->isAppProtocolConnected())
    {
      if (0 != s_mqttClientWrapper)
      {
        TR_PRINTF(trPort(), DbgTrace_Level::debug, "MqttClientController::loop(), s_mqttClientWrapper->loop()");
        s_mqttClientWrapper->loop();
        TR_PRINTF(trPort(), DbgTrace_Level::debug, "MqttClientController::loop(), s_mqttClientWrapper->loop() done");
      }
    }
  }
}

int MqttClientController::publish(const char* topic, const char* data)
{
  int ret = 0;
  if (0 != s_mqttClientWrapper)
  {
    TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::debug, "publish(%s, %s)\n", topic, data);
    ret = s_mqttClientWrapper->publish(topic, data);
  }
  return ret;
}

int MqttClientController::subscribe(const char* topic)
{
  int ret = 0;
  if (0 != s_mqttClientWrapper)
  {
    ret = s_mqttClientWrapper->subscribe(topic);
  }
  return ret;
}

int MqttClientController::unsubscribe(const char* topic)
{
  int ret = 0;
  if (0 != s_mqttClientWrapper)
  {
    ret = s_mqttClientWrapper->unsubscribe(topic);
  }
  return ret;
}

MqttTopicSubscriber* MqttClientController::mqttSubscriberChain()
{
  return m_mqttSubscriberChain;
}

MqttTopicPublisher* MqttClientController::mqttPublisherChain()
{
  return m_mqttPublisherChain;
}

void MqttClientController::addMqttSubscriber(MqttTopicSubscriber* mqttSubscriber)
{
  if (0 == m_mqttSubscriberChain)
  {
    m_mqttSubscriberChain = mqttSubscriber;
    TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::info, "Added first MQTT Subscriber: %s", mqttSubscriber->getTopicString());
  }
  else
  {
    MqttTopicSubscriber* next = m_mqttSubscriberChain;
    while (next->next() != 0)
    {
      next = next->next();
    }
    next->setNext(mqttSubscriber);
    TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::info, "Added MQTT Subscriber: %s", mqttSubscriber->getTopicString());
  }
}

void MqttClientController::addMqttPublisher(MqttTopicPublisher* mqttPublisher)
{
  if (0 == m_mqttPublisherChain)
  {
    m_mqttPublisherChain = mqttPublisher;
    TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::info, "Added first MQTT Publisher: %s", mqttPublisher->getTopicString());
  }
  else
  {
    MqttTopicPublisher* next = m_mqttPublisherChain;
    while (next->next() != 0)
    {
      next = next->next();
    }
    next->setNext(mqttPublisher);
    TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::info, "Added MQTT Publisher: %s", mqttPublisher->getTopicString());
  }
}

MqttTopicSubscriber* MqttClientController::findSubscriberByTopic(const char* topic)
{
  MqttTopicSubscriber* subscriber = m_mqttSubscriberChain;
  bool found = false;
  while ((0 != subscriber) && (!found))
  {
    found = (strncmp(subscriber->getTopicString(), topic, strlen(topic)) == 0);
    if (!found)
    {
      subscriber = subscriber->next();
    }
  }
  return subscriber;
}

MqttTopicPublisher* MqttClientController::findPublisherByTopic(const char* topic)
{
  MqttTopicPublisher* publisher = m_mqttPublisherChain;
  bool found = false;
  while ((0 != publisher) && (!found))
  {
    found = (strncmp(publisher->getTopicString(), topic, strlen(topic)) == 0);
    if (!found)
    {
      publisher = publisher->next();
    }
  }
  return publisher;
}

void MqttClientController::deleteSubscriber(MqttTopicSubscriber* subscriberToDelete)
{
  if (m_mqttSubscriberChain == subscriberToDelete)
  {
    m_mqttSubscriberChain = subscriberToDelete->next();
  }
  else
  {
    MqttTopicSubscriber* next = m_mqttSubscriberChain;
    while ((next != 0) && (next->next() != subscriberToDelete))
    {
      next = next->next();
    }
    if (next != 0)
    {
      next->setNext(subscriberToDelete->next());
    }
  }
}

void MqttClientController::deletePublisher(MqttTopicPublisher* publisherToDelete)
{
  if (m_mqttPublisherChain == publisherToDelete)
  {
    m_mqttPublisherChain = publisherToDelete->next();
  }
  else
  {
    MqttTopicPublisher* next = m_mqttPublisherChain;
    while ((next != 0) && (next->next() != publisherToDelete))
    {
      next = next->next();
    }
    if (next != 0)
    {
      next->setNext(publisherToDelete->next());
    }
  }
}

