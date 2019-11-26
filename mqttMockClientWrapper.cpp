/*
 * mqttMockClientWrapper.cpp
 *
 *  Created on: 21.06.2018
 *      Author: nid
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include "MqttTopic.h"
#include <MqttClientController.h>

#include <MqttMockClientWrapper.h>

IMqttClientWrapper* MqttMockClientWrapper::s_mqttMockClientWrapper = 0;

MqttMockClientWrapper::MqttMockClientWrapper(Client& lanClient)
: m_client(lanClient)
, m_callbackAdapter(0)
, m_isConnected(false)
{
  s_mqttMockClientWrapper = this;
}

MqttMockClientWrapper::~MqttMockClientWrapper()
{ }

void mqttMockClientCallback(const char* topic, const char* payload, unsigned int length)
{
  IMqttClientWrapper* mqttMockWrapper = MqttMockClientWrapper::s_mqttMockClientWrapper;
  if (0 != mqttMockWrapper)
  {
    IMqttClientCallbackAdapter* callbackAdapter = mqttMockWrapper->callbackAdapter();
    if (0 != callbackAdapter)
    {
      callbackAdapter->messageReceived(topic, payload, length);
    }
  }
}

void MqttMockClientWrapper::setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter)
{
  m_callbackAdapter = callbackAdapter;
}

IMqttClientCallbackAdapter* MqttMockClientWrapper::callbackAdapter()
{
  return m_callbackAdapter;
}

Client& MqttMockClientWrapper::client()
{
  return m_client;
}

void MqttMockClientWrapper::setServer(const char* domain, uint16_t port)
{ }

void MqttMockClientWrapper::setClient(Client& client)
{
  m_client = client;
}

bool MqttMockClientWrapper::connect(const char* id)
{
  m_isConnected = true;
  return m_isConnected;
}

void MqttMockClientWrapper::disconnect()
{
  m_isConnected = false;
}

bool MqttMockClientWrapper::connected()
{
  return m_isConnected;
}

bool MqttMockClientWrapper::loop()
{
  return true;
}

unsigned char MqttMockClientWrapper::publish(const char* topic, const char* data)
{
  unsigned char ret = 0;
  return ret;
}

unsigned char MqttMockClientWrapper::subscribe(const char* topic)
{
  unsigned char ret = 0;
  return ret;
}

unsigned char MqttMockClientWrapper::unsubscribe(const char* topic)
{
  unsigned char ret = 0;
  return ret;
}

IMqttClientWrapper::eIMqttClientState MqttMockClientWrapper::state()
{
  eIMqttClientState iMqttClientState = eIMqttCS_ConnectUnavailable;
  return iMqttClientState;
};

//-----------------------------------------------------------------------------

MqttMockClientCallbackAdapter::MqttMockClientCallbackAdapter()
: m_trPortMqttRx(new DbgTrace_Port("mqttrx", DbgTrace_Level::info))
, m_rxMsg(new MqttRxMsg())
{ }

MqttMockClientCallbackAdapter::~MqttMockClientCallbackAdapter()
{
  delete m_rxMsg;
  m_rxMsg = 0;

  delete m_trPortMqttRx;
  m_trPortMqttRx = 0;
}

void MqttMockClientCallbackAdapter::messageReceived(const char* topic, const char* payload, unsigned int length)
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
