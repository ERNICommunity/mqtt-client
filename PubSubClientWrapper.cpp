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
      callbackAdapter->messageReceived(topic, payload, length);
    }
  }
}

void PubSubClientWrapper::setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter)
{
  m_callbackAdapter = callbackAdapter;
  m_pubSubClient->setCallback(pubSubClientCallback);
}

IMqttClientCallbackAdapter* PubSubClientWrapper::callbackAdapter()
{
  return m_callbackAdapter;
}

Client& PubSubClientWrapper::client()
{
  return m_client;
}


bool PubSubClientWrapper::connect(const char* id)
{
  return m_pubSubClient->connect(id);
}

void PubSubClientWrapper::disconnect()
{
  m_pubSubClient->disconnect();
}

bool PubSubClientWrapper::connected()
{
  return m_pubSubClient->connected();
}

bool PubSubClientWrapper::loop()
{
  return m_pubSubClient->loop();
}

unsigned char PubSubClientWrapper::publish(const char* topic, const char* data)
{
  return m_pubSubClient->publish(topic, data);
}

unsigned char PubSubClientWrapper::subscribe(const char* topic)
{
  return m_pubSubClient->subscribe(topic);
}

unsigned char PubSubClientWrapper::unsubscribe(const char* topic)
{
  return m_pubSubClient->unsubscribe(topic);
}

IMqttClientWrapper::eIMqttClientState PubSubClientWrapper::state()
{
  int pubSubClientState = m_pubSubClient->state();
  eIMqttClientState iMqttClientState = eIMqttCS_Disconnected;

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

void PubSubClientCallbackAdapter::messageReceived(char* topic, unsigned char* payload, unsigned int length)
{
  char msg[length+1];
  memcpy(msg, payload, length);
  msg[length] = 0;

  TR_PRINTF(m_trPortMqttRx, DbgTrace_Level::info, "Message arrived, topic: %s - msg: %s", topic, msg);

  m_rxMsg->prepare(topic, payload, length);

  MqttTopicSubscriber* mqttSubscriberChain = MqttClientController::Instance()->mqttSubscriberChain();
  if (0 != mqttSubscriberChain)
  {
    mqttSubscriberChain->handleMessage(m_rxMsg, m_trPortMqttRx);
  }
}
