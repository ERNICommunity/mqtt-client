/*
 * PubSubClientWrapper.h
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_

#include <IMqttClientWrapper.h>
#include <MqttClientController.h>

class PubSubClient;
class DbgTrace_Port;
class MqttRxMsg;

class PubSubClientWrapper: public IMqttClientWrapper
{
public:
  PubSubClientWrapper(Client& lanClient, const char* mqttServerAddr, unsigned short int mqttPort = MqttClientController::defaultMqttPort);
  virtual ~PubSubClientWrapper();

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
  static IMqttClientWrapper* s_pubSubClientWrapper;

private:
  Client& m_client;
  PubSubClient* m_pubSubClient;
  IMqttClientCallbackAdapter* m_callbackAdapter;

private: // forbidden default functions
  PubSubClientWrapper();                                            // default constructor
  PubSubClientWrapper& operator = (const PubSubClientWrapper& src); // assignment operator
  PubSubClientWrapper(const PubSubClientWrapper& src);              // copy constructor
};

//-----------------------------------------------------------------------------

class PubSubClientCallbackAdapter : public IMqttClientCallbackAdapter
{
private:
  DbgTrace_Port* m_trPortMqttRx;
  MqttRxMsg*     m_rxMsg;

public:
  PubSubClientCallbackAdapter();
  virtual ~PubSubClientCallbackAdapter();
  void messageReceived(char* topic, unsigned char* payload, unsigned int length);

private: // forbidden default functions
  PubSubClientCallbackAdapter& operator = (const PubSubClientCallbackAdapter& src); // assignment operator
  PubSubClientCallbackAdapter(const PubSubClientCallbackAdapter& src);              // copy constructor

};

#endif /* LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_ */
