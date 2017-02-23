/*
 * MqttMsgHandler.cpp
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#include <Arduino.h>
#include <string.h>

#include "MqttMsgHandler.h"

#include "MqttClientController.h"

const unsigned int MqttMsgHandler::s_maxRxTopicSize = 100;
const unsigned int MqttMsgHandler::s_maxRxMsgSize   = 500;

MqttMsgHandler::MqttMsgHandler(const char* topic)
: m_topic(new char[strlen(topic)+1])
, m_rxTopic(new char[s_maxRxTopicSize+1])
, m_rxMsg(new char[s_maxRxMsgSize+1])
, m_next(0)
{
  memset(m_topic, 0, strlen(topic)+1);
  strcpy(m_topic, topic);

  memset(m_rxTopic, 0, s_maxRxTopicSize+1);
  memset(m_rxMsg,   0, s_maxRxMsgSize+1);
}

MqttMsgHandler::~MqttMsgHandler()
{
  delete [] m_topic;
  m_topic = 0;

  delete [] m_rxMsg;
  m_rxMsg = 0;

  delete [] m_rxTopic;
  m_rxTopic = 0;
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

bool MqttMsgHandler::isMyTopic() const
{
  bool ismytopic = false;
  if (strncmp(m_topic, m_rxTopic, s_maxRxTopicSize) == 0)
  {
    ismytopic = true;
  }
  return ismytopic;
}

const char* MqttMsgHandler::getTopic() const
{
  return m_topic;
}

const char* MqttMsgHandler::getRxTopic() const
{
  return m_rxTopic;
}

const char* MqttMsgHandler::getRxMsg() const
{
  return m_rxMsg;
}

void MqttMsgHandler::handleMessage(const char* topic, unsigned char* payload, unsigned int length)
{
  if (length > s_maxRxMsgSize)
  {
    length = s_maxRxMsgSize;
  }
  memcpy(m_rxMsg, payload, length);
  m_rxMsg[length] = 0;

  unsigned int len = strlen(topic);
  if (len > s_maxRxTopicSize)
  {
    len = s_maxRxTopicSize;
  }
  memcpy(m_rxTopic, 0, len);
  m_rxTopic[len] = 0;

  // TODO nid: trPort
//  Serial.print("LED test handler, topic: ");
//  Serial.print(getTopic());
//  Serial.print(", msg: ");
//  Serial.println(msg);

  bool msgHasBeenHandled = processMessage();
  if (!msgHasBeenHandled)
  {
    if (0 != next())
    {
      next()->handleMessage(topic, payload, length);
    }
  }
}

void MqttMsgHandler::subscribe()
{
  MqttClientController::Instance()->subscribe(getTopic());
  if (0 != next())
  {
    next()->subscribe();
  }
}

MqttMsgHandler* MqttMsgHandler::next()
{
  return m_next;
}

//-----------------------------------------------------------------------------

DefaultMqttMsgHandler::DefaultMqttMsgHandler(const char* topic)
: MqttMsgHandler(topic)
{ }

bool DefaultMqttMsgHandler::processMessage()
{
  bool msgHasBeenHandled = false;

  if (isMyTopic())
  {
    msgHasBeenHandled = true;

    // take responsibility
    Serial.print("DEFAULT handler, msg rx, topic: ");
    Serial.print(getTopic());
    Serial.print(", msg: ");
    Serial.println(getRxMsg());
  }

  return msgHasBeenHandled;
}

