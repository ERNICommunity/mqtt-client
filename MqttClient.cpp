/*
 * MqttClient.cpp
 *
 *  Created on: 11.02.2017
 *      Author: nid
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#endif
#include "MqttClient.h"
#include <MqttClientController.h>
#include <MqttMsgHandler.h>
#include <PubSubClientWrapper.h>


const unsigned short int MqttClient::s_defaultMqttPort = 1883;

MqttClient::MqttClient(const char* mqttServerAddr, unsigned short int mqttPort, Client* lanClient)
: m_clientController(MqttClientController::Instance())
{
  if (0 == lanClient)
  {
#ifdef ESP8266
    lanClient = new WiFiClient();
#endif
  }

  if (0 != lanClient)
  {
    m_clientController->assignMqttClientWrapper(new PubSubClientWrapper(*(lanClient), mqttServerAddr), new PubSubClientCallbackAdapter());
    m_clientController->setShallConnect(true);
  }
  else
  {
    m_clientController = 0;
  }
}



MqttClient::~MqttClient()
{ }

void MqttClient::setShallConnect(bool shallConnect)
{
  if (0 != m_clientController)
  {
    m_clientController->setShallConnect(shallConnect);
  }
}

bool MqttClient::getShallConnect()
{
  bool shallConnect = false;
  if (0 != m_clientController)
  {
    shallConnect = m_clientController->getShallConnect();
  }
  return shallConnect;
}

void MqttClient::loop()
{
  if (0 != m_clientController)
  {
    m_clientController->loop();
  }
}

int MqttClient::publish(const char* topic, const char* data)
{
  int result = 0;
  if (0 != m_clientController)
  {
    result = m_clientController->publish(topic, data);
  }
  return result;
}

int MqttClient::subscribe(MqttMsgHandler* mqttMsgHandler)
{
  int result = 0;
  if (0 != m_clientController)
  {
    result = m_clientController->subscribe(mqttMsgHandler);
  }
  return result;
}

int MqttClient::unsubscribe(const char* topic)
{
  int result = 0;
  if (0 != m_clientController)
  {
    result = m_clientController->unsubscribe(topic);
  }
  return result;
}
