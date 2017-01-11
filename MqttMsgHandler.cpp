/*
 * MqttMsgHandler.cpp
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#include <Arduino.h>
#include <string.h>

#include "MqttMsgHandler.h"

MqttMsgHandler::MqttMsgHandler(const char* topic)
: m_topic(0)
, m_next(0)
{
  m_topic = new char[strlen(topic) + 1];
  strcpy(m_topic, topic);
}

MqttMsgHandler::~MqttMsgHandler()
{
  delete [] m_topic;
  m_topic = 0;
}

void MqttMsgHandler::addHandler(MqttMsgHandler* handler)
{
  if (0 == m_next)
  {
    m_next = handler;
  }
  else
  {
    m_next->addHandler(handler);
  }
}

bool MqttMsgHandler::isMyTopic(const char* topic)
{
  bool ismytopic = false;
  if (strcmp(m_topic, topic) == 0)
  {
    ismytopic = true;
  }
  return ismytopic;
}

const char* MqttMsgHandler::getTopic()
{
  return m_topic;
}

MqttMsgHandler* MqttMsgHandler::next()
{
  return m_next;
}

//-----------------------------------------------------------------------------

DefaultMqttMsgHandler::DefaultMqttMsgHandler(const char* topic)
: MqttMsgHandler(topic)
{ }

void DefaultMqttMsgHandler::handleMessage(const char* topic, byte* payload, unsigned int length)
{
  if (isMyTopic(topic))
  {
    // take responsibility
    char msg[length+1];
    memcpy(msg, payload, length);
    msg[length] = 0;

    Serial.print("DEFAULT handler, msg rx, topic: ");
    Serial.print(getTopic());
    Serial.print(", msg: ");
    Serial.println(msg);
  }
  else if (0 != next())
  {
    // delegate
    next()->handleMessage(topic, payload, length);
  }
  else
  {
    Serial.println("DEFAULT handler is the last in the chain");
  }
}

