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
class PubSubClientWrapper;
class Timer;
class ConnectionMonitor;
class DbgTrace_Port;
class MqttMsgHandler;

class MqttClientController
{
  friend class MqttClientCtrlReconnectTimerAdapter;

private:
  MqttClientController();
public:
  static MqttClientController* Instance();

  virtual ~MqttClientController();

  static void assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper, IMqttClientCallbackAdapter* mqttClientCallbackAdapter);
  static IMqttClientWrapper* mqttClientWrapper();

  IMqttClientWrapper* mqttClientWrapper();

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  void loop();

  int publish(const char* topic, const char* data);
  int subscribe(const char* topic);
  int subscribe(MqttMsgHandler* mqttMsgHandler);
  int unsubscribe(const char* topic);

  void connect();

  ConnectionMonitor* connMon();
  DbgTrace_Port* trPort();

  MqttMsgHandler* msgHandlerChain();

private:
  void addMsgHandler(MqttMsgHandler* handler);

private:
  static MqttClientController* s_instance;
  static IMqttClientWrapper*   s_mqttClientWrapper;
  bool m_shallConnect;
  DbgTrace_Port* m_trPortMqttctrl;
  ConnectionMonitor* m_connMon;
  MqttMsgHandler* m_handlerChain;

private: // forbidden default functions
  MqttClientController& operator = (const MqttClientController& src); // assignment operator
  MqttClientController(const MqttClientController& src);              // copy constructor
};


#endif /* SRC_MQTTCLIENTCONTROLLER_H_ */
