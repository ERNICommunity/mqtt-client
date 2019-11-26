/*
 * mqttMockClientWrapper.h
 *
 *  Created on: 21.06.2018
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MQTTMOCKCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_MQTTMOCKCLIENTWRAPPER_H_

#include <IMqttClientWrapper.h>
#include <MqttClientController.h>

class DbgTrace_Port;
class MqttRxMsg;

class MqttMockClientWrapper: public IMqttClientWrapper
{
public:
  MqttMockClientWrapper(Client& lanClient);
  virtual ~MqttMockClientWrapper();

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
  static IMqttClientWrapper* s_mqttMockClientWrapper;
  static void (*callback)(char*, uint8_t*, unsigned int);

private:
  Client& m_client;
  IMqttClientCallbackAdapter* m_callbackAdapter;
  bool m_isConnected;

private: // forbidden default functions
  MqttMockClientWrapper();                                              // default constructor
  MqttMockClientWrapper& operator = (const MqttMockClientWrapper& src); // assignment operator
  MqttMockClientWrapper(const MqttMockClientWrapper& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class MqttMockClientCallbackAdapter : public IMqttClientCallbackAdapter
{
private:
  DbgTrace_Port* m_trPortMqttRx;
  MqttRxMsg*     m_rxMsg;

public:
  MqttMockClientCallbackAdapter();
  virtual ~MqttMockClientCallbackAdapter();
  void messageReceived(const char* topic, const char* payload, unsigned int length);

private: // forbidden default functions
  MqttMockClientCallbackAdapter& operator = (const MqttMockClientCallbackAdapter& src); // assignment operator
  MqttMockClientCallbackAdapter(const MqttMockClientCallbackAdapter& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTMOCKCLIENTWRAPPER_H_ */
