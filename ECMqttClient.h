/*
 * ECMqttClient.h
 *
 *  Created on: 23.06.2017
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_MODEL_ERNICOMMUNITYMQTTCLIENT_H_
#define LIB_MQTT_CLIENT_MODEL_ERNICOMMUNITYMQTTCLIENT_H_

class Client;

class ECMqttClientClass
{
public:
  static const unsigned short int defaultMqttPort;
  static const unsigned short int defaultSecureMqttPort;
  ECMqttClientClass() { }
  virtual ~ECMqttClientClass() { }
  void begin(const char* address, unsigned short int port, Client& client, const char* id);
  void loop();

private: // forbidden default functions
  ECMqttClientClass& operator = (const ECMqttClientClass& src); // assignment operator
  ECMqttClientClass(const ECMqttClientClass& src);              // copy constructor
};

extern ECMqttClientClass ECMqttClient;

#endif /* LIB_MQTT_CLIENT_MODEL_ERNICOMMUNITYMQTTCLIENT_H_ */
