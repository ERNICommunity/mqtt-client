/*
 * MqttClientWrapper.cpp
 *
 *  Created on: 22.06.2018
 *      Author: nid
 */

#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <MQTT.h>
#include <MqttTopic.h>
#include <MqttClientController.h>
#include <MqttClientWrapper.h>

IMqttClientWrapper* MqttClientWrapper::s_mqttClientWrapper = 0;

MqttClientWrapper::MqttClientWrapper(Client& lanClient)
: m_client(lanClient)
, m_mqttClient(new MQTTClient(256))
, m_callbackAdapter(0)
{
  s_mqttClientWrapper = this;
}

MqttClientWrapper::~MqttClientWrapper()
{ }

void mqttClientCallback(String& topic, String& payload)
{
  IMqttClientWrapper* mqttWrapper = MqttClientWrapper::s_mqttClientWrapper;
  if (0 != mqttWrapper)
  {
    IMqttClientCallbackAdapter* callbackAdapter = mqttWrapper->callbackAdapter();
    if (0 != callbackAdapter)
    {
      callbackAdapter->messageReceived(topic.c_str(), payload.c_str(), payload.length());
    }
  }
}

void MqttClientWrapper::setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter)
{
  m_callbackAdapter = callbackAdapter;
  if (0 != m_mqttClient)
  {
    m_mqttClient->onMessage(mqttClientCallback);
  }

}

IMqttClientCallbackAdapter* MqttClientWrapper::callbackAdapter()
{
  return m_callbackAdapter;
}

Client& MqttClientWrapper::client()
{
  return m_client;
}

void MqttClientWrapper::setServer(const char* domain, uint16_t port)
{
  if (0 != m_mqttClient)
  {
    m_mqttClient->begin(domain, port, m_client);
  }
}

void MqttClientWrapper::setClient(Client& client)
{ }

bool MqttClientWrapper::connect(const char* id)
{
  bool isConnected = false;
  if (0 != m_mqttClient)
  {
    isConnected = m_mqttClient->connect(id);
  }
  return isConnected;
}

void MqttClientWrapper::disconnect()
{
  if (0 != m_mqttClient)
  {
    m_mqttClient->disconnect();
  }
}

bool MqttClientWrapper::connected()
{
  bool isConnected = false;
  if (0 != m_mqttClient)
  {
    isConnected = m_mqttClient->connected();
  }
  return isConnected;
}

bool MqttClientWrapper::loop()
{
  bool isConnected = false;
  if (0 != m_mqttClient)
  {
    isConnected = m_mqttClient->loop();
  }
  return isConnected;
}

unsigned char MqttClientWrapper::publish(const char* topic, const char* data)
{
  unsigned char ret = 0;
  if (0 != m_mqttClient)
  {
    ret = m_mqttClient->publish(topic, data);
  }
  return ret;
}

unsigned char MqttClientWrapper::subscribe(const char* topic)
{
  unsigned char ret = 0;
  if (0 != m_mqttClient)
  {
    ret = m_mqttClient->subscribe(topic);
  }
  return ret;
}

unsigned char MqttClientWrapper::unsubscribe(const char* topic)
{
  unsigned char ret = 0;
  if (0 != m_mqttClient)
  {
    ret = m_mqttClient->unsubscribe(topic);
  }
  return ret;
}

IMqttClientWrapper::eIMqttClientState MqttClientWrapper::state()
{
  // TODO
  eIMqttClientState iMqttClientState = eIMqttCS_ConnectUnavailable;
  return iMqttClientState;
};

//-----------------------------------------------------------------------------

MqttClientCallbackAdapter::MqttClientCallbackAdapter()
: m_trPortMqttRx(new DbgTrace_Port("mqttrx", DbgTrace_Level::info))
, m_rxMsg(new MqttRxMsg())
{ }

MqttClientCallbackAdapter::~MqttClientCallbackAdapter()
{
  delete m_rxMsg;
  m_rxMsg = 0;

  delete m_trPortMqttRx;
  m_trPortMqttRx = 0;
}

void MqttClientCallbackAdapter::messageReceived(const char* topic, const char* payload, unsigned int length)
{
  char msg[length+1];
  memcpy(msg, payload, length);
  msg[length] = 0;

  TR_PRINTF(m_trPortMqttRx, DbgTrace_Level::info, "Message arrived, topic: %s - msg: %s (len: %d)", topic, msg, length);

  if (0 != m_rxMsg)
  {
    m_rxMsg->prepare(topic, payload, length);
  }

  MqttTopicSubscriber* mqttSubscriberChain = MqttClientController::Instance()->mqttSubscriberChain();
  if (0 != mqttSubscriberChain)
  {
    mqttSubscriberChain->handleMessage(m_rxMsg, m_trPortMqttRx);
  }
}
