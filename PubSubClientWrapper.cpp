/*
 * PubSubClientWrapper.cpp
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <PubSubClient.h>
#include "MqttTopic.h"
#include <MqttClientController.h>

#include <PubSubClientWrapper.h>

IMqttClientWrapper* PubSubClientWrapper::s_pubSubClientWrapper = 0;

PubSubClientWrapper::PubSubClientWrapper(Client& lanClient, const char* mqttServerAddr, unsigned short int mqttPort)
: m_client(lanClient)
, m_pubSubClient(new PubSubClient(mqttServerAddr, mqttPort, lanClient))
, m_callbackAdapter(0)
{
  s_pubSubClientWrapper = this;
}

PubSubClientWrapper::~PubSubClientWrapper()
{ }

void pubSubClientCallback(char* topic, unsigned char* payload, unsigned int length)
{
  IMqttClientWrapper* pubSubWrapper = PubSubClientWrapper::s_pubSubClientWrapper;
  if (0 != pubSubWrapper)
  {
    IMqttClientCallbackAdapter* callbackAdapter = pubSubWrapper->callbackAdapter();
    if (0 != callbackAdapter)
    {
      String myTopic(topic);
      String myPayload(reinterpret_cast<const char*>(payload));
      callbackAdapter->messageReceived(myTopic.c_str(), myPayload.c_str(), length);
    }
  }
}

void PubSubClientWrapper::setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter)
{
  m_callbackAdapter = callbackAdapter;
  if (0 != m_pubSubClient)
  {
    m_pubSubClient->setCallback(pubSubClientCallback);
  }
}

IMqttClientCallbackAdapter* PubSubClientWrapper::callbackAdapter()
{
  return m_callbackAdapter;
}

Client& PubSubClientWrapper::client()
{
  return m_client;
}

void PubSubClientWrapper::setServer(const char* domain, uint16_t port)
{
  if (0 != m_pubSubClient)
  {
    m_pubSubClient->setServer(domain, port);
  }
}

void PubSubClientWrapper::setClient(Client& client)
{
  if (0 != m_pubSubClient)
  {
    m_pubSubClient->setClient(client);
  }
}

bool PubSubClientWrapper::connect(const char* id)
{
  bool isConnected = false;
  if (0 != m_pubSubClient)
  {
    isConnected = m_pubSubClient->connect(id);
  }
  return isConnected;
}

void PubSubClientWrapper::disconnect()
{
  if (0 != m_pubSubClient)
  {
    m_pubSubClient->disconnect();
  }
}

bool PubSubClientWrapper::connected()
{
  bool isConnected = false;
  if (0 != m_pubSubClient)
  {
    isConnected = m_pubSubClient->connected();
    yield();
  }
  return isConnected;
}

bool PubSubClientWrapper::loop()
{
  bool isConnected = false;
  if (0 != m_pubSubClient)
  {
    yield();
    isConnected = m_pubSubClient->loop();
  }
  return isConnected;
}

unsigned char PubSubClientWrapper::publish(const char* topic, const char* data)
{
  unsigned char ret = 0;
  if (0 != m_pubSubClient)
  {
    yield();
    ret = m_pubSubClient->publish(topic, data);
  }
  return ret;
}

unsigned char PubSubClientWrapper::subscribe(const char* topic)
{
  unsigned char ret = 0;
  if (0 != m_pubSubClient)
  {
    yield();
    ret = m_pubSubClient->subscribe(topic);
  }
  return ret;
}

unsigned char PubSubClientWrapper::unsubscribe(const char* topic)
{
  unsigned char ret = 0;
  if (0 != m_pubSubClient)
  {
    yield();
    ret = m_pubSubClient->unsubscribe(topic);
  }
  return ret;
}

IMqttClientWrapper::eIMqttClientState PubSubClientWrapper::state()
{
  eIMqttClientState iMqttClientState = eIMqttCS_ConnectUnavailable;

  if (0 != m_pubSubClient)
  {
    yield();
    int pubSubClientState = m_pubSubClient->state();

    switch (pubSubClientState)
    {
      case MQTT_CONNECTION_TIMEOUT       : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectionTimeout    ; break;
      case MQTT_CONNECTION_LOST          : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectionLost       ; break;
      case MQTT_CONNECT_FAILED           : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectFailed        ; break;
      case MQTT_DISCONNECTED             : iMqttClientState = IMqttClientWrapper::eIMqttCS_Disconnected         ; break;
      case MQTT_CONNECTED                : iMqttClientState = IMqttClientWrapper::eIMqttCS_Connected            ; break;
      case MQTT_CONNECT_BAD_PROTOCOL     : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadProtocol   ; break;
      case MQTT_CONNECT_BAD_CLIENT_ID    : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadClientId   ; break;
      case MQTT_CONNECT_UNAVAILABLE      : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectUnavailable   ; break;
      case MQTT_CONNECT_BAD_CREDENTIALS  : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadCredentials; break;
      case MQTT_CONNECT_UNAUTHORIZED     : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectUnauthorized  ; break;
      default: break;
    }
  }

  return iMqttClientState;
};

//-----------------------------------------------------------------------------

PubSubClientCallbackAdapter::PubSubClientCallbackAdapter()
: m_trPortMqttRx(new DbgTrace_Port("mqttrx", DbgTrace_Level::info))
, m_rxMsg(new MqttRxMsg())
{ }

PubSubClientCallbackAdapter::~PubSubClientCallbackAdapter()
{
  delete m_rxMsg;
  m_rxMsg = 0;

  delete m_trPortMqttRx;
  m_trPortMqttRx = 0;
}

void PubSubClientCallbackAdapter::messageReceived(const char* topic, const char* payload, unsigned int length)
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
