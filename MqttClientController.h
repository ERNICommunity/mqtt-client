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
class PubSubClientWrapper;
class Timer;

//const unsigned short int defaultMqttPort = 1883;

class MqttClientController
{
  friend class MqttClientCtrlReconnectTimerAdapter;

private:
  MqttClientController();
public:
  static MqttClientController* Instance();
public:
  virtual ~MqttClientController();

  static void assignMqttClientWrapper(IMqttClientWrapper* mqttClientWrapper);

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  void loop();

  int publish(const char* topic, const char* data);
  int subscribe(const char* topic);

protected:
  void connect();
  void reconnect();

private:
  static MqttClientController* s_instance;
  static IMqttClientWrapper*   s_mqttClientWrapper;
  Timer* m_reconnectTimer;
  bool m_isConnected;

private: // forbidden default functions
  MqttClientController& operator = (const MqttClientController& src); // assignment operator
  MqttClientController(const MqttClientController& src);              // copy constructor
};


#endif /* SRC_MQTTCLIENTCONTROLLER_H_ */
