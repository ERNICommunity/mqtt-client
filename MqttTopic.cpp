/*
 * MqttTopic.cpp
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#include <MqttTopic.h>

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <MqttClientController.h>

//-----------------------------------------------------------------------------

TopicLevel::TopicLevel(const char* level, unsigned int idx)
: m_idx(idx)
, m_levelSize(strlen(level)+1)
, m_level(new char[m_levelSize])
, m_wcType(eTWC_None)
, m_next(0)
{
  strncpy(m_level, level, m_levelSize);

  if (strncmp(m_level, "+", m_levelSize) == 0)
  {
    m_wcType = eTWC_Single;
  }
  else if (strncmp(level, "#", m_levelSize) == 0)
  {
    m_wcType = eTWC_Multi;
  }
}

TopicLevel::~TopicLevel()
{
  delete m_next;
  m_next = 0;

  delete [] m_level;
  m_level = 0;
}

void TopicLevel::append(TopicLevel* level)
{
  if (0 == m_next)
  {
    m_next = level;
  }
  else
  {
    m_next->append(level);
  }
}

TopicLevel* TopicLevel::next()
{
  return m_next;
}

const char* TopicLevel::level() const
{
  return m_level;
}

unsigned int TopicLevel::idx() const
{
  return m_idx;
}

TopicLevel::WildcardType TopicLevel::getWildcardType()
{
  return m_wcType;
}

//-----------------------------------------------------------------------------

const unsigned int MqttTopic::s_maxNumOfTopicLevels = 25;

MqttTopic::MqttTopic(const char* topic)
: m_topic(new char[strlen(topic)+1])
, m_topicLevelCount(0)
, m_levelList(0)
, m_hasWildcards(false)
{
  memset(m_topic, 0, strlen(topic)+1);
  strncpy(m_topic, topic, strlen(topic));

  char tmpTopic[strlen(topic)+1];
  memset(tmpTopic, 0, strlen(topic)+1);
  strncpy(tmpTopic, topic, strlen(topic));

  unsigned int i = 0;
  TopicLevel* levelObj = 0;
  char* level = strtok(tmpTopic, "/");
  while (level != 0)
  {
    levelObj = new TopicLevel(level, i);
    m_hasWildcards |= TopicLevel::eTWC_None != levelObj->getWildcardType();
    appendLevel(levelObj);
    level = strtok(0, "/");
    i++;
  }
  m_topicLevelCount = i;

//  Serial.print("Topic: ");
//  Serial.print(m_topic);
//  Serial.print(", #levels: ");
//  Serial.print(m_topicLevelCount);
//  Serial.print(", has ");
//  Serial.print(!hasWildcards() ? "no " : "");
//  Serial.println("Wildcards");

//  levelObj = getLevelList();
//  while(0 != levelObj)
//  {
//    Serial.print("[");
//    Serial.print(levelObj->idx());
//    Serial.print("] - ");
//    Serial.println(levelObj->level());
//    levelObj = levelObj->next();
//  }
}

MqttTopic::~MqttTopic()
{
  delete m_levelList;
  m_levelList = 0;

  delete [] m_topic;
  m_topic = 0;
}

const char* MqttTopic::getTopicString() const
{
  return m_topic;
}

bool MqttTopic::hasWildcards() const
{
  return m_hasWildcards;
}

void MqttTopic::appendLevel(TopicLevel* level)
{
  if (0 == m_levelList)
  {
    m_levelList = level;
  }
  else
  {
    m_levelList->append(level);
  }
}

TopicLevel* MqttTopic::getLevelList() const
{
  return m_levelList;
}

//-----------------------------------------------------------------------------

const unsigned int MqttTopicPublisher::s_maxDataSize = 500;
const bool MqttTopicPublisher::DO_AUTO_PUBLISH = true;
const bool MqttTopicPublisher::DONT_AUTO_PUBLISH = false;

MqttTopicPublisher::MqttTopicPublisher(const char* topic, const char* data, bool isAutoPublish)
: MqttTopic(topic)
, m_next(0)
, m_data(new char[s_maxDataSize+1])
, m_isAutoPublish(isAutoPublish)
{
  memset(m_data, 0, s_maxDataSize+1);
  strncpy(m_data, data, s_maxDataSize);
  MqttClientController::Instance()->addMqttPublisher(this);
  if (m_isAutoPublish)
  {
    int r = 0;
    r = MqttClientController::Instance()->publish(getTopicString(), m_data);
  }
}

MqttTopicPublisher::~MqttTopicPublisher()
{
  MqttClientController::Instance()->deletePublisher(this);
}

void MqttTopicPublisher::setData(const char* data)
{
  memset(m_data, 0, s_maxDataSize+1);
  strncpy(m_data, data, s_maxDataSize);
}

const char* MqttTopicPublisher::getData() const
{
  return m_data;
}

void MqttTopicPublisher::publish(const char* data)
{
  int r = 0;
  r = MqttClientController::Instance()->publish(getTopicString(), data);
}

void MqttTopicPublisher::publish()
{
  int r = 0;
  r = MqttClientController::Instance()->publish(getTopicString(), m_data);
}

void MqttTopicPublisher::publishAll()
{
  if (m_isAutoPublish)
  {
    int r = 0;
    r = MqttClientController::Instance()->publish(getTopicString(), m_data);
  }
  if (0 != next())
  {
    yield();
    next()->publishAll();
  }
}

void MqttTopicPublisher::setNext(MqttTopicPublisher* mqttPublisher)
{
  m_next = mqttPublisher;
}

MqttTopicPublisher* MqttTopicPublisher::next()
{
  return m_next;
}

//-----------------------------------------------------------------------------

const unsigned int MqttRxMsg::s_maxRxMsgSize        = 500;

MqttRxMsg::MqttRxMsg()
: m_rxTopic(0)
, m_rxMsg(new char[s_maxRxMsgSize+1])
, m_rxMsgSize(0)
{
  memset(m_rxMsg, 0, s_maxRxMsgSize+1);
}

MqttRxMsg::~MqttRxMsg()
{
  delete [] m_rxMsg;
  m_rxMsg = 0;

  delete m_rxTopic;
  m_rxTopic = 0;
}

void MqttRxMsg::prepare(const char* topic, const char* payload, unsigned int length)
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

  delete m_rxTopic;
  m_rxTopic = new MqttTopic(topic);
 }

MqttTopic* MqttRxMsg::getRxTopic() const
{
  return m_rxTopic;
}

const char* MqttRxMsg::getRxMsgString() const
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
{
  MqttClientController::Instance()->addMqttSubscriber(this);
  MqttClientController::Instance()->subscribe(topic);
}

MqttTopicSubscriber::~MqttTopicSubscriber()
{
  MqttClientController::Instance()->unsubscribe(getTopicString());
  MqttClientController::Instance()->deleteSubscriber(this);
}

void MqttTopicSubscriber::setNext(MqttTopicSubscriber* mqttSubscriber)
{
  m_next = mqttSubscriber;
}

bool MqttTopicSubscriber::isMyTopic() const
{
  bool ismytopic = false;
  if ((0 != m_rxMsg) && (0 != m_rxMsg->getRxTopic()))
  {
    if (hasWildcards())
    {
      // handle smart compare
      bool stillMatch = true;
      TopicLevel* subscriberTopicLevel = getLevelList();
      TopicLevel* rxTopicLevel = m_rxMsg->getRxTopic()->getLevelList();
      while(stillMatch && (0 != subscriberTopicLevel) && (0 != rxTopicLevel))
      {
        if (TopicLevel::eTWC_None == subscriberTopicLevel->getWildcardType())
        {
          stillMatch &= (strcmp(subscriberTopicLevel->level(), rxTopicLevel->level()) == 0);
        }
        subscriberTopicLevel = subscriberTopicLevel->next();
        rxTopicLevel = rxTopicLevel->next();
      }
      ismytopic = stillMatch;
    }
    else
    {
      if (strcmp(getTopicString(), m_rxMsg->getRxTopic()->getTopicString()) == 0)
      {
        ismytopic = true;
      }
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
  if ((0 != trPortMqttRx) && (0 != m_rxMsg) && (0 != m_rxMsg->getRxTopic()))
  {
    TR_PRINTF(trPortMqttRx, DbgTrace_Level::debug, "MqttTopicSubscriber::handleMessage(), topic: %s, rx topic: %s, rx msg: %s", getTopicString(), m_rxMsg->getRxTopic()->getTopicString(), m_rxMsg->getRxMsgString());
  }

  bool msgHasBeenHandled = processMessage();
  if (!msgHasBeenHandled)
  {
    if (0 != next())
    {
      yield();
      next()->handleMessage(rxMsg, trPortMqttRx);
    }
  }
}

void MqttTopicSubscriber::subscribe()
{
  MqttClientController::Instance()->subscribe(getTopicString());
  if (0 != next())
  {
    next()->subscribe();
    yield();
  }
}

MqttTopicSubscriber* MqttTopicSubscriber::next()
{
  return m_next;
}

//-----------------------------------------------------------------------------

DefaultMqttSubscriber::DefaultMqttSubscriber(const char* topic)
: MqttTopicSubscriber(topic)
, m_trPort(new DbgTrace_Port("mqttdfltsub", DbgTrace_Level::debug))
{ }

bool DefaultMqttSubscriber::processMessage()
{
  bool msgHasBeenHandled = false;
  MqttRxMsg* rxMsg = getRxMsg();

  if (isMyTopic() && (0 != rxMsg))
  {
    // take responsibility
    msgHasBeenHandled = true;
    TR_PRINTF(m_trPort, DbgTrace_Level::debug, "DefaultMqttSubscriber (%s), rx: [%s] %s", getTopicString(), rxMsg->getRxTopic()->getTopicString(), rxMsg->getRxMsgString());
  }

  return msgHasBeenHandled;
}

//-----------------------------------------------------------------------------

DefaultMqttPublisher::DefaultMqttPublisher(const char* topic, const char* data)
: MqttTopicPublisher(topic, data)
, m_trPort(new DbgTrace_Port("mqttdfltpub", DbgTrace_Level::debug))
{ }

void DefaultMqttPublisher::publish(const char* data)
{
  TR_PRINTF(m_trPort, DbgTrace_Level::debug, "DefaultMqttPublisher (%s), tx: %s", getTopicString(), getData());
  MqttTopicPublisher::publish(data);
}



