/*
 * IMqttClientWrapper.h
 *
 *  Created on: 18.10.2016
 *      Author: nid
 */

#ifndef LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_
#define LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_

class IMqttClientCallbackAdapter
{
protected:
  IMqttClientCallbackAdapter() { }

public:
  virtual ~IMqttClientCallbackAdapter() { }

  virtual void messageReceived(char* topic, byte* payload, unsigned int length) = 0;
};

class IMqttClientWrapper
{
protected:
  IMqttClientWrapper() { }

public:
  virtual ~IMqttClientWrapper() { }

  /**
   * Set LAN Client.
   * @param lanClient Pointer to the LAN client object to be used for TCP/IP communication.
   */
  virtual void setClient(Client* lanClient) = 0;

  /**
   * Set MQTT Broker server address and port.
   * @param mqttServerAddr Server address.
   * @param mqttPort MQTT port.
   */
  virtual void setServer(const char* mqttServerAddr, unsigned short int mqttPort) = 0;

  /**
   * Set callback adapter.
   * @param IMqttClientCallbackAdapter Pointer to an object implementing the interface to the MQTT Client Callback Adapter.
   */
  virtual void setCallbackAdapter(IMqttClientCallbackAdapter* callbackAdapter) = 0;

  virtual IMqttClientCallbackAdapter* callbackAdapter() = 0;

  /**
   * Process MQTT messages.
   * To be called in the Arduino loop() function.
   */
  virtual void processMessages() = 0;

  virtual bool connect(const char* id) = 0;
  virtual void disconnect() = 0;
  virtual bool connected() = 0;

  /**
   * Publish data in a message to the mentioned MQTT Topic.
   * @param topic The MQTT topic to publish to
   * @param data  The data to be published
   */
  virtual int publish(const char* topic, const char* data) = 0;

  /**
   * Subscribe the mentioned MQTT Topic.
   * @param topic The MQTT topic to publish to
   */
  virtual int subscribe(const char* topic) = 0;

  typedef enum
  {
    eIMqttCS_Connected              =  0,
    eIMqttCS_ConnectBadProtocol     =  1,
    eIMqttCS_ConnectBadClientId     =  2,
    eIMqttCS_ConnectUnavailable     =  3,
    eIMqttCS_ConnectBadCredentials  =  4,
    eIMqttCS_ConnectUnauthorized    =  5,
    eIMqttCS_ConnectionTimeout      = 12,
    eIMqttCS_ConnectionLost         = 13,
    eIMqttCS_ConnectFailed          = 14,
    eIMqttCS_Disconnected           = 15
  } eIMqttClientState;

  virtual eIMqttClientState state() = 0;

private: // forbidden default functions
  IMqttClientWrapper& operator = (const IMqttClientWrapper& src); // assignment operator
  IMqttClientWrapper(const IMqttClientWrapper& src);              // copy constructor
};

#endif /* LIB_MQTT_CLIENT_IMQTTCLIENTWRAPPER_H_ */
