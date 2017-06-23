/*
 * MqttClient.h
 *
 *  Created on: 23.06.2017
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MODEL_MQTTCLIENT_H_
#define LIB_MQTT_CLIENT_MODEL_MQTTCLIENT_H_

class MqttClientClass
{
public:
  static const unsigned short int defaultMqttPort;
  void begin(const char* address, unsigned short int port = defaultMqttPort);
};

extern MqttClientClass MqttClient;

#endif /* LIB_MQTT_CLIENT_MODEL_MQTTCLIENT_H_ */
