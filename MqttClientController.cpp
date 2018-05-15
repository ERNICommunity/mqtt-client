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

  void notifyLanConnected(bool isLanConnected)
  {
    if (isLanConnected && (0 != m_mqttClientCtrl) && (0 != m_mqttClientCtrl->connMon()))
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "LAN Connection: ON");
      if (m_mqttClientCtrl->getShallConnect() && !m_mqttClientCtrl->connMon()->isAppProtocolConnected())
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

  void notifyAppProtocolConnected(bool isMqttConnected)
  {
    if (isMqttConnected)
    {
      TR_PRINT_STR(trPort(), DbgTrace_Level::debug, "MQTT Connection: ON");

      // subscribe to topics
      MqttTopicSubscriber* subscriberChain = m_mqttClientCtrl->mqttSubscriberChain();
      if (0 != subscriberChain)
      {
        subscriberChain->subscribe();
      }

      // publish the auto publisher topics
      MqttTopicPublisher* publisherChain = m_mqttClientCtrl->mqttPublisherChain();
      if (0 != publisherChain)
      {
        publisherChain->publishAll();
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
    assignMqttClientWrapper(new PubSubClientWrapper(*(lanClient), "test.mosquitto.org"), new PubSubClientCallbackAdapter());
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
  s_mqttClientWrapper->setCallbackAdapter(mqttClientCallbackAdapter);
}


IMqttClientWrapper* MqttClientController::mqttClientWrapper()
{
  return s_mqttClientWrapper;
}

void MqttClientController::setServer(const char* domain, uint16_t port)
{
  s_mqttClientWrapper->setServer(domain, port);
}

void MqttClientController::setClient(Client& client)
{
  s_mqttClientWrapper->setClient(client);
}


void MqttClientController::setShallConnect(bool shallConnect)
{
  m_shallConnect = shallConnect;
  m_connMon->setAppProtocolState(shallConnect);
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
  if (m_connMon->isAppProtocolConnected())
  {
    bool mqttIsConnected = s_mqttClientWrapper->loop();
    m_connMon->setAppProtocolState(mqttIsConnected);
  }
}

int MqttClientController::publish(const char* topic, const char* data)
{
  TR_PRINTF(m_trPortMqttctrl, DbgTrace_Level::debug, "publish(%s, %s)\n", topic, data);
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

