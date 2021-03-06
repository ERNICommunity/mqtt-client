/*
 * MqttClientController.h
 *
 *  Created on: 13.10.2016
 *      Author: nid
 */

#ifndef SRC_MQTTCLIENTCONTROLLER_H_
#define SRC_MQTTCLIENTCONTROLLER_H_

class Client;
class IMqttClientWrapper;
class IMqttClientCallbackAdapter;
class Timer;
class ConnMon;
class DbgTrace_Port;
class MqttTopicSubscriber;
class MqttTopicPublisher;

class MqttClientController
{
  friend class MqttClientCtrlReconnectTimerAdapter;
  friend class MqttTopicPublisher;
  friend class MqttTopicSubscriber;

private:
  MqttClientController();
public:
  static MqttClientController* Instance();

  virtual ~MqttClientController();

  static void assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper, IMqttClientCallbackAdapter* mqttClientCallbackAdapter);
  static IMqttClientWrapper* mqttClientWrapper();

  void setServer(const char* domain, unsigned short int port=defaultMqttPort);
  void setClient(Client& client);

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  void loop();

protected:
  int publish(const char* topic, const char* data);
  int subscribe(const char* topic);
  int unsubscribe(const char* topic);

public:
  bool connect();

  ConnMon* connMon();
  DbgTrace_Port* trPort();

  MqttTopicSubscriber* mqttSubscriberChain();
  MqttTopicPublisher* mqttPublisherChain();

  MqttTopicSubscriber* findSubscriberByTopic(const char* topic);
  MqttTopicPublisher*  findPublisherByTopic(const char* topic);

protected:
  void deleteSubscriber(MqttTopicSubscriber* subscriberToDelete);
  void deletePublisher(MqttTopicPublisher* publisherToDelete);
  void addMqttSubscriber(MqttTopicSubscriber* mqttSubscriber);
  void addMqttPublisher(MqttTopicPublisher* mqttPublisher);

public:
  static const unsigned short int defaultMqttPort;

private:
  static MqttClientController* s_instance;
  static IMqttClientWrapper*   s_mqttClientWrapper;

  bool m_shallConnect;
  DbgTrace_Port* m_trPortMqttctrl;
  ConnMon* m_connMon;
  MqttTopicSubscriber* m_mqttSubscriberChain;
  MqttTopicPublisher* m_mqttPublisherChain;

private: // forbidden default functions
  MqttClientController& operator = (const MqttClientController& src); // assignment operator
  MqttClientController(const MqttClientController& src);              // copy constructor
};


#endif /* SRC_MQTTCLIENTCONTROLLER_H_ */
