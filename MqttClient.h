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
class MqttTopicSubscriber;

/**
 * MQTT Client Library API class.
 */
class MqttClient
{
public:
  /**
   * Constructor. Automatically starts trying to connect to the given Broker.
   * @param mqttServerAddr MQTT broker address.
   * @param mqttPort MQTT port (default 1883)
   * @param lanClient (default: WiFi Client will be created
   */
  MqttClient(const char* mqttServerAddr, unsigned short int mqttPort = s_defaultMqttPort, Client* lanClient = 0);


  /**
   * Destructor.
   */
  virtual ~MqttClient();

  void setShallConnect(bool shallConnect);
  bool getShallConnect();

  /**
   * Kick function to be called from Arduino Framework loop() function in order to keep the message processing going.
   */
  void loop();

  /**
   * Publish the data to the specified topic.
   * @param topic where to publish to
   * @param data to be published
   * @return
   */
  int publish(const char* topic, const char* data);
  int subscribe(MqttTopicSubscriber* mqttSubscriber);
  int unsubscribe(const char* topic);

private:
  MqttClientController* m_clientController;
  static const unsigned short int s_defaultMqttPort; /* = 1883;*/


private: // forbidden default functions
  MqttClient();                                   // default constructor
  MqttClient& operator = (const MqttClient& src); // assignment operator
  MqttClient(const MqttClient& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_MQTTCLIENT_H_ */
