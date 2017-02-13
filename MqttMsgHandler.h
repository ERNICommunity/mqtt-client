/*
 * MqttMsgHandler.h
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_
#define LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_

/**
 * Abstract handler for received MQTT messages from subscribed topics.
 * This is an implementation according the GOF Chain-of-responsibility pattern Pattern
 */
class MqttMsgHandler
{
public:
  MqttMsgHandler(const char* topic);
  virtual ~MqttMsgHandler();

  virtual void handleMessage(const char* topic, unsigned char* payload, unsigned int length) = 0;

  void addHandler(MqttMsgHandler* handler);

  bool isMyTopic(const char* topic);

  const char* getTopic();

  void subscribe();

  MqttMsgHandler* next();

private:
  char* m_topic;
  MqttMsgHandler* m_next;

private:
  // forbidden default functions
  MqttMsgHandler();                                       // default constructor
  MqttMsgHandler& operator = (const MqttMsgHandler& src); // assignment operator
  MqttMsgHandler(const MqttMsgHandler& src);              // copy constructor
};

//-----------------------------------------------------------------------------

/**
 * Default implementation of a message handler.
 */
class DefaultMqttMsgHandler : public MqttMsgHandler
{
public:
  DefaultMqttMsgHandler(const char* topic);

  virtual void handleMessage(const char* topic, unsigned char* payload, unsigned int length);

private:
  // forbidden default functions
  DefaultMqttMsgHandler();                                              // default constructor
  DefaultMqttMsgHandler& operator = (const DefaultMqttMsgHandler& src); // assignment operator
  DefaultMqttMsgHandler(const DefaultMqttMsgHandler& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_ */
