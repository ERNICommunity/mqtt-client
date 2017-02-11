/*
 * MqttClient.h
 *
 *  Created on: 11.02.2017
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTCLIENT_H_
#define LIB_MQTT_CLIENT_MQTTCLIENT_H_

class Client;
class MqttClientController;
class MqttMsgHandler;

class MqttClient
{
public:
  MqttClient(const char* mqttServerAddr, unsigned short int mqttPort = s_defaultMqttPort, Client* lanClient = 0);
  virtual ~MqttClient();

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  void loop();

  int publish(const char* topic, const char* data);
  int subscribe(MqttMsgHandler* mqttMsgHandler);
  int unsubscribe(const char* topic);

private:
  MqttClientController* m_clientController;
  static const unsigned short int s_defaultMqttPort; /* = 1883;*/


private: // forbidden default functions
//  MqttClient();                                   // default constructor
  MqttClient& operator = (const MqttClient& src); // assignment operator
  MqttClient(const MqttClient& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTCLIENT_H_ */
