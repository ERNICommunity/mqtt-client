/*
 * PubSubClientWrapper.cpp
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#include <PubSubClient.h>

#include <PubSubClientWrapper.h>

PubSubClientWrapper* PubSubClientWrapper::s_pubSubClientWrapper = 0;

PubSubClientWrapper::PubSubClientWrapper()
: m_pubSubClient(new PubSubClient())
, m_callbackAdapter(0)
{
  s_pubSubClientWrapper = this;
}

PubSubClientWrapper::~PubSubClientWrapper()
{
  // TODO Auto-generated destructor stub
}

void PubSubClientWrapper::setClient(Client* lanClient)
{
  m_pubSubClient->setClient(*(lanClient));
}

void PubSubClientWrapper::setServer(const char* mqttServerAddr, unsigned short int mqttPort)
{
  m_pubSubClient->setServer(mqttServerAddr, mqttPort);
}

void pubSubClientCallback(char* topic, byte* payload, unsigned int length)
{
  if ((0 != PubSubClientWrapper::s_pubSubClientWrapper) && (0 != PubSubClientWrapper::s_pubSubClientWrapper->callbackAdapter()))
  {
    PubSubClientWrapper::s_pubSubClientWrapper->callbackAdapter()->messageReceived(topic, payload, length);
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
  m_pubSubClient->connected();
}

void PubSubClientWrapper::processMessages()
{
  m_pubSubClient->loop();
}

int PubSubClientWrapper::publish(const char* topic, const char* data)
{
  return m_pubSubClient->publish(topic, data);
}

int PubSubClientWrapper::subscribe(const char* topic)
{
  m_pubSubClient->subscribe(topic);
}

IMqttClientWrapper::eIMqttClientState PubSubClientWrapper::state()
{
  int pubSubClientState = m_pubSubClient->state();
  eIMqttClientState iMqttClientState = eIMqttCS_Disconnected;

  switch (pubSubClientState)
  {
    case MQTT_CONNECTION_TIMEOUT       : iMqttClientState = IMqttClientWrapper::eIMqttCS_Connected             ; break;
    case MQTT_CONNECTION_LOST          : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadProtocol    ; break;
    case MQTT_CONNECT_FAILED           : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadClientId    ; break;
    case MQTT_DISCONNECTED             : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectUnavailable    ; break;
    case MQTT_CONNECTED                : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectBadCredentials ; break;
    case MQTT_CONNECT_BAD_PROTOCOL     : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectUnauthorized   ; break;
    case MQTT_CONNECT_BAD_CLIENT_ID    : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectionTimeout     ; break;
    case MQTT_CONNECT_UNAVAILABLE      : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectionLost        ; break;
    case MQTT_CONNECT_BAD_CREDENTIALS  : iMqttClientState = IMqttClientWrapper::eIMqttCS_ConnectFailed         ; break;
    case MQTT_CONNECT_UNAUTHORIZED     : iMqttClientState = IMqttClientWrapper::eIMqttCS_Disconnected          ; break;
    default: break;
  }

  return iMqttClientState;
};


