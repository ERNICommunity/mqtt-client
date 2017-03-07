/*
 * MqttMsgHandler.h
 *
 *  Created on: 16.12.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_
#define LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_

class DbgTrace_Port;

/**
 * Abstract handler for received MQTT messages from subscribed topics.
 * This is an implementation according the GOF Chain-of-responsibility pattern Pattern
 */
class MqttMsgHandler
{
public:
  MqttMsgHandler(const char* topic);
  virtual ~MqttMsgHandler();

  void handleMessage(const char* topic, unsigned char* payload, unsigned int length, DbgTrace_Port* trPortMqttRx = 0);

  virtual bool processMessage() = 0;

  void addHandler(MqttMsgHandler* handler);

  bool isMyTopic() const;

  const char* getTopic() const;

  const char* getRxTopic() const;

  const char* getRxMsg() const;

  void subscribe();

  MqttMsgHandler* next();

private:
  char* m_topic;
  char* m_rxTopic;
  char* m_rxMsg;
  static const unsigned int s_maxRxTopicSize;
  static const unsigned int s_maxRxMsgSize;
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

  virtual bool processMessage();

private:
  // forbidden default functions
  DefaultMqttMsgHandler();                                              // default constructor
  DefaultMqttMsgHandler& operator = (const DefaultMqttMsgHandler& src); // assignment operator
  DefaultMqttMsgHandler(const DefaultMqttMsgHandler& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTMSGHANDLER_H_ */
