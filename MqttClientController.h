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
class LanConnectionMonitor;
class DbgTrace_Port;

class MqttClientController
{
  friend class MqttClientCtrlReconnectTimerAdapter;

private:
  MqttClientController();
public:
  static MqttClientController* Instance();

  virtual ~MqttClientController();

  static void assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper, IMqttClientCallbackAdapter* mqttClientCallbackAdapter);

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  void loop();

  int publish(const char* topic, const char* data);
  int subscribe(const char* topic);
  int unsubscribe(const char* topic);

  void connect();
  void reconnect();

  LanConnectionMonitor* lanConnMon();
  DbgTrace_Port* trPort();

private:
  static MqttClientController* s_instance;
  static IMqttClientWrapper*   s_mqttClientWrapper;
  Timer* m_reconnectTimer;
  DbgTrace_Port* m_trPortMqttctrl;
  LanConnectionMonitor* m_lanConnMon;

  bool m_isConnected;

private: // forbidden default functions
  MqttClientController& operator = (const MqttClientController& src); // assignment operator
  MqttClientController(const MqttClientController& src);              // copy constructor
};


#endif /* SRC_MQTTCLIENTCONTROLLER_H_ */
