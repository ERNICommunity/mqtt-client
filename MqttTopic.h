/*
 * MqttTopic.h
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTTOPIC_H_
#define LIB_MQTT_CLIENT_MQTTTOPIC_H_

class DbgTrace_Port;

class TopicLevel
{
public:
  TopicLevel(const char* level, unsigned int idx);
  virtual ~TopicLevel();

  TopicLevel* next();
  const char* level() const;
  unsigned int idx() const;

  void append(TopicLevel* level);

  enum WildcardType
  {
    eTWC_None = 0,
    eTWC_Single = 1,
    eTWC_Multi = 2
  };

  WildcardType getWildcardType();

private:
  const unsigned int m_idx;
  const unsigned int m_levelSize;
  char* m_level;
  WildcardType m_wcType;
  TopicLevel* m_next;
private:
  // forbidden default functions
  TopicLevel();                                   // default constructor
  TopicLevel& operator = (const TopicLevel& src); // assignment operator
  TopicLevel(const TopicLevel& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class MqttTopic
{
public:
  MqttTopic(const char* topic);
  virtual ~MqttTopic();

  const char* getTopicString() const;
  TopicLevel* getLevelList() const;
  bool hasWildcards() const;

protected:
  void appendLevel(TopicLevel* level);

private:
  char* m_topic;
  unsigned int m_topicLevelCount;
  static const unsigned int s_maxNumOfTopicLevels;
  TopicLevel* m_levelList;
  bool m_hasWildcards;

private:
  // forbidden default functions
  MqttTopic();                                  // default constructor
  MqttTopic& operator = (const MqttTopic& src); // assignment operator
  MqttTopic(const MqttTopic& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class MqttTopicPublisher : public MqttTopic
{
public:
  MqttTopicPublisher(const char* topic, const char* data, bool isAutoPublishOnConnectEnabled = DONT_AUTO_PUBLISH);
  virtual ~MqttTopicPublisher();

  void setData(const char* data);
  void publish(const char* data);
  void publish();

  const char* getData() const;

  void setNext(MqttTopicPublisher* mqttPublisher);
  MqttTopicPublisher* next();

  void publishAll();

private:
  MqttTopicPublisher* m_next;
  char* m_data;
  bool m_isAutoPublish;
  static const unsigned int s_maxDataSize;

public:
  static const bool DO_AUTO_PUBLISH;
  static const bool DONT_AUTO_PUBLISH;

private:
  // forbidden default functions
  MqttTopicPublisher();                                           // default constructor
  MqttTopicPublisher& operator = (const MqttTopicPublisher& src); // assignment operator
  MqttTopicPublisher(const MqttTopicPublisher& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class MqttRxMsg
{
public:
  MqttRxMsg();
  virtual ~MqttRxMsg();

  void prepare(const char* topic, const char* payload, unsigned int length);

  MqttTopic* getRxTopic() const;
  const char* getRxMsgString() const;
  const unsigned int getRxMsgSize() const;

private:
  MqttTopic* m_rxTopic;
  char* m_rxMsg;
  unsigned int m_rxMsgSize;

public:
  static const unsigned int s_maxRxMsgSize;

private:
  // forbidden default functions
  MqttRxMsg& operator = (const MqttRxMsg& src); // assignment operator
  MqttRxMsg(const MqttRxMsg& src);              // copy constructor
};

//-----------------------------------------------------------------------------

/**
 * Abstract handler for received MQTT messages from subscribed topics.
 * This is an implementation according the GOF Chain-of-responsibility pattern Pattern
 */
class MqttTopicSubscriber : public MqttTopic
{
public:
  MqttTopicSubscriber(const char* topic);
  virtual ~MqttTopicSubscriber();

  void handleMessage(MqttRxMsg* rxMsg, DbgTrace_Port* trPortMqttRx = 0);
  virtual bool processMessage(MqttRxMsg* rxMsg) = 0;
  bool isMyTopic(MqttRxMsg* rxMsg) const;
  void subscribe();
  void setNext(MqttTopicSubscriber* mqttSubscriber);
  MqttTopicSubscriber* next();

private:
  MqttTopicSubscriber* m_next;

private:
  // forbidden default functions
  MqttTopicSubscriber();                                            // default constructor
  MqttTopicSubscriber& operator = (const MqttTopicSubscriber& src); // assignment operator
  MqttTopicSubscriber(const MqttTopicSubscriber& src);              // copy constructor
};

//-----------------------------------------------------------------------------

/**
 * Default implementation of an MQTT Topic Subscriber.
 */
class DefaultMqttSubscriber : public MqttTopicSubscriber
{
public:
  DefaultMqttSubscriber(const char* topic);
  virtual bool processMessage(MqttRxMsg* rxMsg);
private:
  DbgTrace_Port* m_trPort;
private:
  // forbidden default functions
  DefaultMqttSubscriber();                                              // default constructor
  DefaultMqttSubscriber& operator = (const DefaultMqttSubscriber& src); // assignment operator
  DefaultMqttSubscriber(const DefaultMqttSubscriber& src);              // copy constructor
};

//-----------------------------------------------------------------------------

/**
 * Default implementation of an MQTT Topic Publisher.
 */
class DefaultMqttPublisher : public MqttTopicPublisher
{
public:
  DefaultMqttPublisher(const char* topic, const char* data);
  void publish(const char* data);
private:
  DbgTrace_Port* m_trPort;
private:
  // forbidden default functions
  DefaultMqttPublisher();                                               // default constructor
  DefaultMqttPublisher& operator = (const DefaultMqttPublisher& src);   // assignment operator
  DefaultMqttPublisher(const DefaultMqttPublisher& src);                // copy constructor
};


#endif /* LIB_MQTT_CLIENT_MQTTTOPIC_H_ */
