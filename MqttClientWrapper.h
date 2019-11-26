/*
 * mqttClientWrapper.h
 *
 *  Created on: 22.06.2018
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_MQTTCLIENTWRAPPER_H_

#include <IMqttClientWrapper.h>
#include <MqttClientController.h>

class MQTTClient;
class DbgTrace_Port;
class MqttRxMsg;

class MqttClientWrapper: public IMqttClientWrapper
{
public:
  MqttClientWrapper(Client& lanClient);
  virtual ~MqttClientWrapper();

  void setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter);
  IMqttClientCallbackAdapter* callbackAdapter();
  Client& client();
  void setServer(const char* domain, uint16_t port=MqttClientController::defaultMqttPort);
  void setClient(Client& client);
  bool loop();
  bool connect(const char* id);
  void disconnect();
  bool connected();
  unsigned char publish(const char* topic, const char* data);
  unsigned char subscribe(const char* topic);
  unsigned char unsubscribe(const char* topic);
  eIMqttClientState state();

public:
  static IMqttClientWrapper* s_mqttClientWrapper;
  static void (*callback)(char*, uint8_t*, unsigned int);

private:
  Client& m_client;
  MQTTClient* m_mqttClient;
  IMqttClientCallbackAdapter* m_callbackAdapter;

private: // forbidden default functions
  MqttClientWrapper();                                          // default constructor
  MqttClientWrapper& operator = (const MqttClientWrapper& src); // assignment operator
  MqttClientWrapper(const MqttClientWrapper& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class MqttClientCallbackAdapter : public IMqttClientCallbackAdapter
{
private:
  DbgTrace_Port* m_trPortMqttRx;
  MqttRxMsg*     m_rxMsg;

public:
  MqttClientCallbackAdapter();
  virtual ~MqttClientCallbackAdapter();
  void messageReceived(const char* topic, const char* payload, unsigned int length);

private: // forbidden default functions
  MqttClientCallbackAdapter& operator = (const MqttClientCallbackAdapter& src); // assignment operator
  MqttClientCallbackAdapter(const MqttClientCallbackAdapter& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTCLIENTWRAPPER_H_ */
