/*
 * MqttMsgHandler.h
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTTOPIC_H_
#define LIB_MQTT_CLIENT_MQTTTOPIC_H_

class DbgTrace_Port;

class MqttTopic
{
public:
  MqttTopic(const char* topic);
  virtual ~MqttTopic();

  const char* getTopic() const;
private:
  char* m_topic;


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
  MqttTopicPublisher(const char* topic);
  virtual ~MqttTopicPublisher();

  virtual void publish() { }
  virtual void publish(const char* msg) { }
  virtual void publish(int val) { }
  virtual void publish(double val) { }

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

  void prepare(const char* topic, unsigned char* payload, unsigned int length);

  const char* getRxTopic() const;
  const char* getRxMsg() const;
  const unsigned int getRxMsgSize() const;

private:
  char* m_rxTopic;
  char* m_rxMsg;
  unsigned int m_rxMsgSize;

public:
  static const unsigned int s_maxRxTopicSize;
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
  virtual bool processMessage() = 0;
  void addMqttSubscriber(MqttTopicSubscriber* mqttSubscriber);
  bool isMyTopic() const;
  void subscribe();
  MqttTopicSubscriber* next();
  MqttRxMsg* getRxMsg() const;

private:
  MqttTopicSubscriber* m_next;
  MqttRxMsg* m_rxMsg;

private:
  // forbidden default functions
  MqttTopicSubscriber();                                       // default constructor
  MqttTopicSubscriber& operator = (const MqttTopicSubscriber& src); // assignment operator
  MqttTopicSubscriber(const MqttTopicSubscriber& src);              // copy constructor
};

//-----------------------------------------------------------------------------

/**
 * Default implementation of a message handler.
 */
class DefaultMqttSubscriber : public MqttTopicSubscriber
{
public:
  DefaultMqttSubscriber(const char* topic);

  virtual bool processMessage();

private:
  // forbidden default functions
  DefaultMqttSubscriber();                                              // default constructor
  DefaultMqttSubscriber& operator = (const DefaultMqttSubscriber& src); // assignment operator
  DefaultMqttSubscriber(const DefaultMqttSubscriber& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTTOPIC_H_ */
