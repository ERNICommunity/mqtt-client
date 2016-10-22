/*
 * PubSubClientWrapper.h
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_

#include "IMqttClientWrapper.h"

class PubSubClient;

class PubSubClientWrapper: public IMqttClientWrapper
{
public:
  PubSubClientWrapper();
  virtual ~PubSubClientWrapper();

  void setClient(Client* lanClient);
  void setServer(const char* mqttServerAddr, unsigned short int mqttPort) ;
  void setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter);
  IMqttClientCallbackAdapter* callbackAdapter();
  void processMessages();
  bool connect(const char* id);
  void disconnect();
  bool connected();
  int publish(const char* topic, const char* data);
  int subscribe(const char* topic);
  eIMqttClientState state();

public:
  static PubSubClientWrapper* s_pubSubClientWrapper;

private:
  PubSubClient* m_pubSubClient;
  IMqttClientCallbackAdapter* m_callbackAdapter;
};

#endif /* LIB_MQTT_CLIENT_PUBSUBCLIENTWRAPPER_H_ */
