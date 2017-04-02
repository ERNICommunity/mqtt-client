/*
 * MqttMsgHandler.cpp
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#include "MqttTopic.h"

#include <Arduino.h>
#include <string.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include "MqttClientController.h"

MqttTopic::MqttTopic(const char* topic)
: m_topic(new char[strlen(topic)+1])
{
  memset(m_topic, 0, strlen(topic)+1);
  strncpy(m_topic, topic, strlen(topic));
}

MqttTopic::~MqttTopic()
{
  delete [] m_topic;
  m_topic = 0;
}

const char* MqttTopic::getTopic() const
{
  return m_topic;
}

//-----------------------------------------------------------------------------

MqttTopicPublisher::MqttTopicPublisher(const char* topic)
: MqttTopic(topic)
{ }

MqttTopicPublisher::~MqttTopicPublisher()
{ }

//-----------------------------------------------------------------------------

const unsigned int MqttRxMsg::s_maxRxTopicSize = 100;
const unsigned int MqttRxMsg::s_maxRxMsgSize   = 500;

MqttRxMsg::MqttRxMsg()
: m_rxTopic(new char[s_maxRxTopicSize+1])
, m_rxMsg(new char[s_maxRxMsgSize+1])
, m_rxMsgSize(0)
{
  memset(m_rxTopic, 0, s_maxRxTopicSize+1);
  memset(m_rxMsg,   0, s_maxRxMsgSize+1);
}

MqttRxMsg::~MqttRxMsg()
{
  delete [] m_rxMsg;
  m_rxMsg = 0;

  delete [] m_rxTopic;
  m_rxTopic = 0;
}

void MqttRxMsg::prepare(const char* topic, unsigned char* payload, unsigned int length)
{
  if (length > s_maxRxMsgSize)
  {
    m_rxMsgSize = s_maxRxMsgSize;
  }
  else
  {
    m_rxMsgSize = length;
  }
  memcpy(m_rxMsg, payload, length);
  m_rxMsg[m_rxMsgSize] = 0;

  unsigned int len = strlen(topic)+1;
  if (len > s_maxRxTopicSize+1)
  {
    len = s_maxRxTopicSize+1;
  }
  strncpy(m_rxTopic, topic, len);
  m_rxTopic[len] = 0;
}

const char* MqttRxMsg::getRxTopic() const
{
  return m_rxTopic;
}

const char* MqttRxMsg::getRxMsg() const
{
  return m_rxMsg;
}

const unsigned int MqttRxMsg::getRxMsgSize() const
{
  return m_rxMsgSize;
}

//-----------------------------------------------------------------------------

MqttTopicSubscriber::MqttTopicSubscriber(const char* topic)
: MqttTopic(topic)
, m_next(0)
, m_rxMsg(0)
{ }

MqttTopicSubscriber::~MqttTopicSubscriber()
{ }

void MqttTopicSubscriber::addMqttSubscriber(MqttTopicSubscriber* mqttSubscriber)
{
  if (0 == m_next)
  {
    m_next = mqttSubscriber;
  }
  else
  {
    m_next->addMqttSubscriber(mqttSubscriber);
  }
}

bool MqttTopicSubscriber::isMyTopic() const
{
  bool ismytopic = false;
  if (0 != m_rxMsg)
  {
    if (strncmp(getTopic(), m_rxMsg->getRxTopic(), MqttRxMsg::s_maxRxTopicSize) == 0)
    {
      ismytopic = true;
    }
  }
  return ismytopic;
}

MqttRxMsg* MqttTopicSubscriber::getRxMsg() const
{
  return m_rxMsg;
}


void MqttTopicSubscriber::handleMessage(MqttRxMsg* rxMsg, DbgTrace_Port* trPortMqttRx)
{
  m_rxMsg = rxMsg;
  if ((0 != trPortMqttRx) && (0 != m_rxMsg))
  {
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, "MqttTopicSubscriber::handleMessage(), topic: ");
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, getTopic());
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, "MqttTopicSubscriber::handleMessage(), rx topic: ");
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, m_rxMsg->getRxTopic());
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, "MqttTopicSubscriber::handleMessage(), rx msg: ");
    TR_PRINT_STR(trPortMqttRx, DbgTrace_Level::debug, m_rxMsg->getRxMsg());
  }

  bool msgHasBeenHandled = processMessage();
  if (!msgHasBeenHandled)
  {
    if (0 != next())
    {
      next()->handleMessage(rxMsg, trPortMqttRx);
    }
  }
}

void MqttTopicSubscriber::subscribe()
{
  MqttClientController::Instance()->subscribe(getTopic());
  if (0 != next())
  {
    next()->subscribe();
  }
}

MqttTopicSubscriber* MqttTopicSubscriber::next()
{
  return m_next;
}

//-----------------------------------------------------------------------------

DefaultMqttSubscriber::DefaultMqttSubscriber(const char* topic)
: MqttTopicSubscriber(topic)
{ }

bool DefaultMqttSubscriber::processMessage()
{
  bool msgHasBeenHandled = false;
  MqttRxMsg* rxMsg = getRxMsg();

  if (isMyTopic() && (0 != rxMsg))
  {
    msgHasBeenHandled = true;

    // take responsibility
    Serial.print("DefaultMqttSubscriber: ");
    Serial.print(rxMsg->getRxTopic());
    Serial.print(", ");
    Serial.println(rxMsg->getRxMsg());
  }

  return msgHasBeenHandled;
}

